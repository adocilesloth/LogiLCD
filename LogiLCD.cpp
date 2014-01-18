/********************************************************************************
Copyright (C) 2014 William Pearson <adocilesloth@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/
//const char levels[] PROGMEM = {0b00000, ..14 times}

#include "LogiLCD.h"
#include "LogitechLcd.h"
#include <boost/thread.hpp>
#include <string>
#include <sstream>

//#include <fstream>

using namespace std;

//ofstream file;

bool done = false;
int lasttime;

bool LoadPlugin()
{
	if(!LogiLcdInit(L"OBS", LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR))
	{
		return true;
	}
	if(LogiLcdIsConnected(LOGI_LCD_TYPE_MONO))
	{
		//call mono thread
		boost::thread LcdMono(Mono); 
	}
	if(LogiLcdIsConnected(LOGI_LCD_TYPE_COLOR))
	{
		//call colour thread
		boost::thread LcdColour(Colour);
	}
	return true;
}

void UnloadPlugin()
{
	//stop Logitech LCD thread
	done = true;
	Sleep(10);
	return;
}

CTSTR GetPluginName()
{
	return TEXT("Logitech LCD");
}

CTSTR GetPluginDescription()
{
	return TEXT("Adds Logitech LCD Monochrome and Colour support");
}

void OnStartStream()
{
	lasttime = OSGetTime();
}

void Mono()
{

	//file.open("C:/Program Files (x86)/OBS/plugins/outfile.txt");
	wstring scene;
	bool miclast = false;
	bool desklast = false;
	bool livelast = false;
	bool altdisplast = false;

	/*//fps and bitrate
	int fps;
	float bitrate;
	wstringstream fpsbyte;

	bool altdisplay = false;
	//dropped frames
	int dropped;
	int total;
	double percent;
	wstringstream frames;*/
	//stream time
	int curtime;
	int sec = -1;
	int min = 0;
	int hour = 0;
	wstringstream stime;
	

	while(!done) //Text line length  is 26 characters
	{
		//mute and deafen buttons
		if(miclast == true &&  LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_1) == false) //button released
		{
			OBSToggleMicMute();
		}
		if(desklast == true && LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_2) == false) //button rleased
		{
			OBSToggleDesktopMute();
		}
		miclast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_1);
		desklast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_2);
		//stream and prieview buttons
		if(livelast == true && LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0) == false)
		{
			OBSStartStopStream();
		}
		/*if(altdisplast == true && LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3) == false)
		{
			if(altdisplay)
			{
				altdisplay = false;
			}
			else
			{
				altdisplay = true;
			}
		}*/
		livelast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0);
		/*altdisplast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3);*/

		//scene information
		scene = L"Scene: ";
		scene.append(OBSGetSceneName());
		wchar_t *name = new wchar_t[scene.length() + 1];
		wcscpy(name, scene.c_str());	//convert to wchar_t
		LogiLcdMonoSetText(1, name);	//print
		delete [] name;					//delete temp crap
		scene = L"";

		if(OBSGetStreaming())	//streaming
		{
			//SendMessage(source, (int)SB_GETTEXT, 1, (LPARAM)recieve);
			if(!OBSGetPreviewOnly())	//and not prieview
			{
				if(OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute||Deaf   live\u25CF");
				}
				else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute|        live\u25CF");
				}
				else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS          |Deaf   live\u25CF");
				}
				else
				{
					LogiLcdMonoSetText(0, L"OBS                  live\u25CF");
				}
			}
			else	//and preview
			{
				if(OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute||Deaf       \u25CF");
				}
				else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute|            \u25CF");
				}
				else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS          |Deaf       \u25CF");
				}
				else
				{
					LogiLcdMonoSetText(0, L"OBS                      \u25CF");
				}
			}

			/*//fps and bitrate
			fpsbyte << L"FPS: ";
			fps = OBSGetCaptureFPS();
			if(fps < 10)
			{
				fpsbyte << L"0";
			}
			fpsbyte << fps << L"      Bitrate: ";
			bitrate = float(OBSGetbytesPerSec());	//get as B/s
			bitrate = (bitrate/1000)*8;		//convert to kb/s
			fpsbyte << int(bitrate);
			wchar_t *fpsbit = new wchar_t[fpsbyte.str().length() +1];
			wcscpy(fpsbit, fpsbyte.str().c_str());		//copy string
			LogiLcdMonoSetText(2, fpsbit);			//print to lcd
			delete [] fpsbit;
			fpsbyte.str(L"");*/

			/*//dropped frames calculations
			frames << L"Dropped Frames: ";
			dropped = OBSGetFramesDropped();
			frames << dropped << L"(";
			total = OBSGetTotalFrames();
			percent = (double(dropped)/total)*100;
			frames << fixed << setprecision(2) << percent << L"%)";*/

			//stream time calculations
			stime << "Stream Uptime: ";
			curtime = OSGetTime();
			if(curtime - lasttime >= 1000)
			{
				sec++;
				lasttime = OSGetTime();
			}
			if(sec == 60)
			{
				min++;
				sec = 0;
			}
			if(min == 60)
			{
				hour++;
				min = 0;
			}

			stime << hour << ":";
			if(min < 10)
			{
				stime << "0";
			}
			stime << min << ":";
			if(sec < 10)
			{
				stime << "0";
			}
			stime << sec;

			/*if(!altdisplay)
			{
				//dropped frames
				wchar_t *droppedframes = new wchar_t[frames.str().length() +1];
				wcscpy(droppedframes, frames.str().c_str());		//copy string
				LogiLcdMonoSetText(3, droppedframes);			//print to lcd
				delete [] droppedframes;
			}
			else
			{*/
				//stream time
				wchar_t *streamtime = new wchar_t[stime.str().length() +1];
				wcscpy(streamtime, stime.str().c_str());		//copy string
				LogiLcdMonoSetText(3, streamtime);			//print to lcd
				delete [] streamtime;
				
			/*}
		frames.str(L"");*/
		stime.str(L"");

		}
		else
		{
			if(OBSGetMicMuted() && OBSGetDesktopMuted())
			{
				LogiLcdMonoSetText(0, L"OBS     Mute||Deaf       \u25CB");
			}
			else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
			{
				LogiLcdMonoSetText(0, L"OBS     Mute|            \u25CB");
			}
			else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
			{
				LogiLcdMonoSetText(0, L"OBS          |Deaf       \u25CB");
			}
			else
			{
				LogiLcdMonoSetText(0, L"OBS                      \u25CB");
			}
			/*LogiLcdMonoSetText(2, L"FPS: --      Bitrate: ----");
			if(!altdisplay)
			{
				LogiLcdMonoSetText(3, L"Dropped Frames: -(-.--%)");
			}
			else
			{*/
				LogiLcdMonoSetText(3, L"Stream Uptime: -:--:--");
			/*}*/
			//reset time variables
			if(sec != -1)
			{
				sec = -1;
			}
			if(min == 0)
			{
				min = 0;
			}
			if(hour != 0)
			{
				hour = 0;
			}
			
		}
		//update screen
		LogiLcdUpdate();
	}
	LogiLcdShutdown();

	return;
}

void Colour()
{
	wstring scene;
	bool leftlast = false;
	bool rightlast = false;
	bool uplast = false;
	bool downlast = false;
	bool oklast = false;
	bool cancellast = false;

	/*//fps and bitrate
	int fps;
	float bitrate;
	wstringstream sfps;
	wstringstream sbyte;
	//dropped frames
	int dropped;
	int total;
	double percent;
	wstringstream frames;*/
	//stream time
	int curtime;
	int sec = -1;
	int min = 0;
	int hour = 0;
	wstringstream stime;

	LogiLcdColorSetTitle(L"OBS", 255, 255, 255);

	while(!done)
	{
		//mute and deafen buttons
		if(leftlast == true &&  LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_LEFT) == false) //button released
		{
			OBSToggleMicMute();
		}
		if(rightlast == true && LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_RIGHT) == false) //button rleased
		{
			OBSToggleDesktopMute();
		}
		leftlast = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_LEFT);
		rightlast = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_RIGHT);
		if(uplast == true &&  LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_UP) == false) //button released
		{
			OBSToggleMicMute();
		}
		if(downlast == true && LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_DOWN) == false) //button rleased
		{
			OBSToggleDesktopMute();
		}
		uplast = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_UP);
		downlast = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_DOWN);
		//stream and prieview buttons
		if(oklast == true &&  LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_OK) == false) //button released
		{
			OBSStartStopStream();
		}
		if(cancellast == true &&  LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_CANCEL) == false) //button released
		{
			OBSStartStopPreview();
		}
		oklast = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_OK);
		cancellast = LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_CANCEL);

		//get scene information
		scene = L"Scene: ";
		scene.append(OBSGetSceneName());
		wchar_t *name = new wchar_t[scene.length() + 1];
		wcscpy(name, scene.c_str());	//convert to wchar_t
		LogiLcdColorSetText(1, name, 255, 255, 255); 
		delete [] name;					//delete temp crap
		scene = L"";

		if(OBSGetStreaming())	//streaming
		{
			//live
			if(OBSGetPreviewOnly())
			{
				LogiLcdColorSetText(0, L"Preview \u25CF", 255, 126, 0);
			}
			else
			{
				LogiLcdColorSetText(0, L"Live \u25CF", 255, 0, 0);
			}

			/*//fps
			sfps << L"FPS: ";
			fps = OBSGetCaptureFPS();
			if(fps < 10)
			{
				sfps << L"0";
			}
			sfps << fps;
			wchar_t *wfps = new wchar_t[sfps.str().length() +1];
			wcscpy(wfps, sfps.str().c_str());				//copy string
			if(fps >= 25 && fps < 30)
			{
				LogiLcdColorSetText(2, wfps, 255, 126, 0);	//print to lcd as yellow
			}
			else if(fps < 25)
			{
				LogiLcdColorSetText(2, wfps, 255, 0, 0);	//print to lcd as red
			}
			else
			{
				LogiLcdColorSetText(2, wfps, 255, 255, 255);	//print to lcd
			}
			delete [] wfps;
			sfps.str(L"");

			//bitrate
			sbyte << L"Bitrate: ";
			bitrate = float(OBSGetbytesPerSec());	//get as B/s
			bitrate = (bitrate/1000)*8;				//convert to kb/s
			sbyte << int(bitrate) << L"kb/s";
			wchar_t *wbit = new wchar_t[sbyte.str().length() +1];
			wcscpy(wbit, sbyte.str().c_str());				//copy string
			LogiLcdColorSetText(3, wbit, 255, 255, 255);	//print to lcd
			delete [] wbit;
			sbyte.str(L"");
			
			//dropped frames
			frames << L"Dropped Frames: ";
			dropped = OBSGetFramesDropped();
			frames << dropped << L"(";
			total = OBSGetTotalFrames();
			percent = (double(dropped)/total)*100;
			frames << fixed << setprecision(2) << percent << L"%)";
			wchar_t *droppedframes = new wchar_t[frames.str().length() +1];
			wcscpy(droppedframes, frames.str().c_str());			//copy string

			if(percent >= 5 && percent < 10)	//between 5% and 10% dropped frames
			{
				LogiLcdColorSetText(4, droppedframes, 225, 126, 0);	//print to lcd as yellow
			}
			else if(percent >= 10)		//over 10% dropped
			{
				LogiLcdColorSetText(4, droppedframes, 225, 0, 0);	//print to lcd as red
			}
			else
			{
				LogiLcdColorSetText(4, droppedframes, 225, 225, 225);	//print to lcd
			}
			delete [] droppedframes;
			frames.str(L"");*/

			//stream time
			stime << "Stream Uptime: ";
			curtime = OSGetTime();
			if(curtime - lasttime >= 1000)
			{
				sec++;
				lasttime = OSGetTime();
			}
			if(sec == 60)
			{
				min++;
				sec = 0;
			}
			if(min == 60)
			{
				hour++;
				min = 0;
			}

			stime << hour << ":";
			if(min < 10)
			{
				stime << "0";
			}
			stime << min << ":";
			if(sec < 10)
			{
				stime << "0";
			}
			stime << sec;
			wchar_t *streamtime = new wchar_t[stime.str().length() +1];
			wcscpy(streamtime, stime.str().c_str());			//copy string
			LogiLcdColorSetText(3, streamtime, 255, 255, 255);	//print to lcd		/*LINE 5 FOR FINAL*/
			delete [] streamtime;
			stime.str(L"");
		}
		else
		{
			LogiLcdColorSetText(0, L"", 0, 0, 0);
			/*LogiLcdColorSetText(2, L"FPS: --", 255, 255, 255);
			LogiLcdColorSetText(3, L"Bitrate: ----kb/s", 255, 255, 255);
			LogiLcdColorSetText(4, L"Dropped Frames: -(-.--%)", 255, 255, 255);*/
			if(sec != -1)
			{
				sec = -1;
			}
			if(min == 0)
			{
				min = 0;
			}
			if(hour != 0)
			{
				hour = 0;
			}
			LogiLcdColorSetText(3, L"Stream Uptime: -:--:--", 255, 255, 255);	/*LINE 5 FOR FINAL*/
		}

		if(OBSGetMicMuted() && OBSGetDesktopMuted())	/*LINE 6 FOR FINAL*/
		{
			LogiLcdColorSetText(4, L"Muted and Deafened", 255, 0, 0);
		}
		else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
		{
			LogiLcdColorSetText(4, L"Muted", 255, 126, 0);
		}
		else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
		{
			LogiLcdColorSetText(4, L"Deafened", 255, 126, 0);
		}
		else
		{
			LogiLcdColorSetText(4, L"", 0, 0, 0);
		}
		//update screen
		LogiLcdUpdate();
	}

	LogiLcdShutdown();
	return;
}
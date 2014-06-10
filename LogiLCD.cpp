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

#include "LogiLCD.h"
#include "LogitechLcd.h"
#include "mono_background.h
#include <string>
#include <sstream>
#include <iomanip>

#include <fstream>

using namespace std;

ofstream file;

ending close;
HANDLE LcdThread;

bool LoadPlugin()
{
	if(!LogiLcdInit(L"OBS", LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR))
	{
		return true;
	}
	if(LogiLcdIsConnected(LOGI_LCD_TYPE_MONO) && LogiLcdIsConnected(LOGI_LCD_TYPE_COLOR))
	{
		//call dual thread
		//LcdThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Dual, NULL, 0, 0);
		LcdThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Colour, NULL, 0, 0);
	}
	else if(LogiLcdIsConnected(LOGI_LCD_TYPE_MONO))
	{
		//call mono thread
		LcdThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Mono, NULL, 0, 0);
	}
	else if(LogiLcdIsConnected(LOGI_LCD_TYPE_COLOR))
	{
		//call colour thread
		LcdThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Colour, NULL, 0, 0);
	}
	return true;
}

void UnloadPlugin()
{
	//stop Logitech LCD thread
	close.now();
	WaitForSingleObject(LcdThread, INFINITE);
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

DWORD WINAPI Mono(LPVOID lpParam)
{
	bool done = false;

	wstring scene;
	bool miclast = false;
	bool desklast = false;
	bool livelast = false;
	bool altdisplast = false;

	//fps and bitrate
	int fps;
	float bitrate;
	wstringstream fpsbyte;

	bool altdisplay = false;
	//dropped frames
	int dropped;
	int total;
	double percent;
	wstringstream frames;
	//stream time
	int uptime;
	int sec;
	int min;
	int hour;
	wstringstream stime;
	
	while(!done) //Text line length  is 26 characters
	{
		//draw mono button help background
		LogiLcdMonoSetBackground(mono_background);
		
		///mute and deafen buttons
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
			HWND streambutton;
			streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Start Streaming"));
			if(!streambutton)	//"Start Streaming" button not found, look for "Stop Streaming" button
			{
				streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Stop Streaming"));
			}

			int brec;
			brec = IsWindowEnabled(streambutton);
			//if either "Start Streaming" or "Stop Streaming" buttons are not found,
			//force OBSStartStopStream() (Will do this for non-English localisation)
			if(!streambutton)
			{
				brec = 1;
			}

			if(brec == 0)
			{
				OBSStartStopRecording();
			}
			else
			{
				OBSStartStopStream();
			}
		}
		if(altdisplast == true && LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3) == false)
		{
			if(altdisplay)
			{
				altdisplay = false;
			}
			else
			{
				altdisplay = true;
			}
		}
		livelast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0);
		altdisplast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3);
		
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
			LogiLcdMonoSetBackground(mono_background_started);
			if(OBSGetPreviewOnly())	//and prieview
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
			else
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
			
			//fps and bitrate
			fpsbyte << L"FPS: ";
			fps = OBSGetCaptureFPS();
			if(fps < 10)
			{
				fpsbyte << L"0";
			}
			fpsbyte << fps << L"      Bitrate: ";
			bitrate = float(OBSGetBytesPerSec());	//get as B/s
			bitrate = (bitrate/1000)*8;		//convert to kb/s
			fpsbyte << int(bitrate);
			wchar_t *fpsbit = new wchar_t[fpsbyte.str().length() +1];
			wcscpy(fpsbit, fpsbyte.str().c_str());		//copy string
			LogiLcdMonoSetText(2, fpsbit);			//print to lcd
			delete [] fpsbit;
			fpsbyte.str(L"");
			
			if(altdisplay)
			{
				//dropped frames
				frames << L"Dropped Frames: ";
				dropped = OBSGetFramesDropped();
				frames << dropped << L"(";
				total = OBSGetTotalFrames();
				percent = (double(dropped)/total)*100;
				frames << fixed << setprecision(2) << percent << L"%)";

				wchar_t *droppedframes = new wchar_t[frames.str().length() +1];
				wcscpy(droppedframes, frames.str().c_str());		//copy string
				LogiLcdMonoSetText(3, droppedframes);			//print to lcd
				delete [] droppedframes;
				frames.str(L"");
			}
			else
			{
				//stream time
				stime << L"Stream Uptime: ";
				uptime = OBSGetTotalStreamTime()/1000;
				hour = (uptime / (60*60)) % 60;
				min = (uptime / 60) % 60;
				sec = (uptime % 60);
				
				stime << hour << L":";
				if(min < 10)
				{
					stime << L"0";
				}
				stime << min << L":";
				if(sec < 10)
				{
					stime << L"0";
				}
				stime << sec;
				
				wchar_t *streamtime = new wchar_t[stime.str().length() +1];
				wcscpy(streamtime, stime.str().c_str());		//copy string
				LogiLcdMonoSetText(3, streamtime);			//print to lcd
				delete [] streamtime;
				stime.str(L"");
			}	
		}//end of streaming
		else
		{
			LogiLcdMonoSetBackground(mono_background_stopped);
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
			LogiLcdMonoSetText(2, L"FPS: --      Bitrate: ----");
			if(altdisplay)
			{
				LogiLcdMonoSetText(3, L"Dropped Frames: -(-.--%)");
			}
			else
			{
				LogiLcdMonoSetText(3, L"Stream Uptime: -:--:--");
			}			
		}
		//update screen
		LogiLcdUpdate();

		Sleep(16);

		if(close.state())
		{
			done = true;
		}
	}
	LogiLcdShutdown();

	return NULL;
}

DWORD WINAPI Colour(LPVOID lpParam)
{
	bool done = false;

	bool live = false;

	wstring scene;
	bool leftlast = false;
	bool rightlast = false;
	bool uplast = false;
	bool downlast = false;
	bool oklast = false;
	bool cancellast = false;

	//fps and bitrate
	int fps;
	float bitrate;
	double fpspercent;
	wstringstream sfps;
	wstringstream sbyte;
	//dropped frames
	int dropped;
	int total;
	double percent;
	wstringstream frames;
	//stream time
	int uptime;
	int sec;
	int min;
	int hour;
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
			HWND streambutton;
			streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Start Streaming"));
			if(!streambutton)
			{
				streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Stop Streaming"));
			}

			int brec;
			brec = IsWindowEnabled(streambutton);
			//if either "Start Streaming" or "Stop Streaming" buttons are not found,
			//force OBSStartStopStream() (Will do this for non-English localisation)
			if(!streambutton)
			{
				brec = 1;
			}

			if(brec == 0)
			{
				OBSStartStopRecording();
			}
			else
			{
				OBSStartStopStream();
			}
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

			//fps
			sfps << L"FPS: ";
			fps = OBSGetCaptureFPS();
			if(fps < 10)
			{
				sfps << L"0";
			}
			sfps << fps;
			wchar_t *wfps = new wchar_t[sfps.str().length() +1];
			wcscpy(wfps, sfps.str().c_str());				//copy string
			fpspercent = double(fps)/OBSGetMaxFPS();
			if(fpspercent >= 0.83 && fpspercent < 0.96)
			{
				LogiLcdColorSetText(2, wfps, 255, 126, 0);	//print to lcd as yellow
			}
			else if(fpspercent < 0.83)
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
			bitrate = float(OBSGetBytesPerSec());	//get as B/s
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
			frames.str(L"");

			//stream time
			stime << L"Stream Uptime: ";
			uptime = OBSGetTotalStreamTime()/1000;
			hour = (uptime / (60*60)) % 60;
			min = (uptime / 60) % 60;
			sec = (uptime % 60);

			stime << hour << L":";
			if(min < 10)
			{
				stime << L"0";
			}
			stime << min << L":";
			if(sec < 10)
			{
				stime << L"0";
			}
			stime << sec;

			wchar_t *streamtime = new wchar_t[stime.str().length() +1];
			wcscpy(streamtime, stime.str().c_str());			//copy string
			LogiLcdColorSetText(5, streamtime, 255, 255, 255);	//print to lcd
			delete [] streamtime;
			stime.str(L"");
		}
		else
		{
			LogiLcdColorSetText(0, L"", 0, 0, 0);
			LogiLcdColorSetText(2, L"FPS: --", 255, 255, 255);
			LogiLcdColorSetText(3, L"Bitrate: ----kb/s", 255, 255, 255);
			LogiLcdColorSetText(4, L"Dropped Frames: -(-.--%)", 255, 255, 255);
			LogiLcdColorSetText(5, L"Stream Uptime: -:--:--", 255, 255, 255);
		}

		if(OBSGetMicMuted() && OBSGetDesktopMuted())
		{
			LogiLcdColorSetText(6, L"Muted and Deafened", 255, 0, 0);
		}
		else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
		{
			LogiLcdColorSetText(6, L"Muted", 255, 126, 0);
		}
		else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
		{
			LogiLcdColorSetText(6, L"Deafened", 255, 126, 0);
		}
		else
		{
			LogiLcdColorSetText(6, L"", 0, 0, 0);
		}
		//update screen
		LogiLcdUpdate();

		Sleep(16);

		if(close.state())
		{
			done = true;
		}
	}

	LogiLcdShutdown();
	return NULL;
}

DWORD WINAPI Dual(LPVOID lpParam)
{
	bool done = false;

	bool live = false;

	wstring scene;
	//mono buttons
	bool miclast = false;
	bool desklast = false;
	bool livelast = false;
	bool altdisplast = false;
	bool altdisplay = false;
	//colour buttons
	bool leftlast = false;
	bool rightlast = false;
	bool uplast = false;
	bool downlast = false;
	bool oklast = false;
	bool cancellast = false;

	//fps and bitrate
	int fps;
	float bitrate;
	double fpspercent;
	wstringstream fpsbyte;
	wstringstream sfps;
	wstringstream sbyte;
	//dropped frames
	int dropped;
	int total;
	double percent;
	wstringstream frames;
	
	//stream time
	int uptime;
	int sec;
	int min;
	int hour;
	wstringstream stime;

	LogiLcdColorSetTitle(L"OBS", 255, 255, 255);

	while(!done)
	{
		//draw mono button help background
		LogiLcdMonoSetBackground(mono_background);
		
		//mono mute and deafen buttons
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
		//mono stream and prieview buttons
		if(livelast == true && LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0) == false)
		{
			HWND streambutton;
			streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Start Streaming"));
			if(!streambutton)
			{
				streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Stop Streaming"));
			}

			int brec;
			brec = IsWindowEnabled(streambutton);
			//if either "Start Streaming" or "Stop Streaming" buttons are not found,
			//force OBSStartStopStream() (Will do this for non-English localisation)
			if(!streambutton)
			{
				brec = 1;
			}

			if(brec == 0)
			{
				OBSStartStopRecording();
			}
			else
			{
				OBSStartStopStream();
			}
		}
		if(altdisplast == true && LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3) == false)
		{
			if(altdisplay)
			{
				altdisplay = false;
			}
			else
			{
				altdisplay = true;
			}
		}
		livelast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0);
		altdisplast = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3);
		//colour mute and deafen buttons
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
		//colour stream and preview buttons
		if(oklast == true &&  LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_OK) == false) //button released
		{
			HWND streambutton;
			streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Start Streaming"));
			if(!streambutton)
			{
				streambutton = FindWindowEx(OBSGetMainWindow(), NULL, TEXT("Button"), TEXT("Stop Streaming"));
			}

			int brec;
			brec = IsWindowEnabled(streambutton);
			//if either "Start Streaming" or "Stop Streaming" buttons are not found,
			//force OBSStartStopStream() (Will do this for non-English localisation)
			if(!streambutton)
			{
				brec = 1;
			}


			if(brec == 0)
			{
				OBSStartStopRecording();
			}
			else
			{
				OBSStartStopStream();
			}
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
		LogiLcdMonoSetText(1, name);
		LogiLcdColorSetText(1, name, 255, 255, 255); 
		delete [] name;					//delete temp crap
		scene = L"";

		if(OBSGetStreaming())	//streaming
		{
			LogiLcdMonoSetBackground(mono_background_started);
			if(OBSGetPreviewOnly())	//and prieview
			{
				LogiLcdColorSetText(0, L"Preview \u25CF", 255, 126, 0);

				if(OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute||Deaf       \u25CF");
					LogiLcdColorSetText(6, L"Muted and Deafened", 255, 0, 0);
				}
				else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute|            \u25CF");
					LogiLcdColorSetText(6, L"Muted", 255, 126, 0);
				}
				else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS          |Deaf       \u25CF");
					LogiLcdColorSetText(6, L"Deafened", 255, 126, 0);
				}
				else
				{
					LogiLcdMonoSetText(0, L"OBS                      \u25CF");
					LogiLcdColorSetText(6, L"", 0, 0, 0);
				}
			}
			else
			{
				LogiLcdColorSetText(0, L"Live \u25CF", 255, 0, 0);

				if(OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute||Deaf   live\u25CF");
					LogiLcdColorSetText(6, L"Muted and Deafened", 255, 0, 0);
				}
				else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS     Mute|        live\u25CF");
					LogiLcdColorSetText(6, L"Muted", 255, 126, 0);
				}
				else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
				{
					LogiLcdMonoSetText(0, L"OBS          |Deaf   live\u25CF");
					LogiLcdColorSetText(6, L"Deafened", 255, 126, 0);
				}
				else
				{
					LogiLcdMonoSetText(0, L"OBS                  live\u25CF");
					LogiLcdColorSetText(6, L"", 0, 0, 0);
				}
			}

			//fps and bitrate
			fpsbyte << L"FPS: ";
			sfps << L"FPS: ";
			fps = OBSGetCaptureFPS();
			if(fps < 10)
			{
				fpsbyte << L"0";
				sfps << L"0";
			}
			fpsbyte << fps << L"      Bitrate: ";
			sfps << fps;
			sbyte << L"Bitrate: ";
			bitrate = float(OBSGetBytesPerSec());	//get as B/s
			bitrate = (bitrate/1000)*8;		//convert to kb/s
			fpsbyte << int(bitrate);
			sbyte << int(bitrate) << L"kb/s";
			//mono fps and bitrate
			wchar_t *fpsbit = new wchar_t[fpsbyte.str().length() +1];
			wcscpy(fpsbit, fpsbyte.str().c_str());		//copy string
			//colour fps
			wchar_t *wfps = new wchar_t[sfps.str().length() +1];
			wcscpy(wfps, sfps.str().c_str());				//copy string
			//colour bitrate
			wchar_t *wbit = new wchar_t[sbyte.str().length() +1];
			wcscpy(wbit, sbyte.str().c_str());				//copy string
			//print to LCDs
			LogiLcdMonoSetText(2, fpsbit);			//print to mono lcd
			fpspercent = double(fps)/OBSGetMaxFPS();
			if(fpspercent >= 0.83 && fpspercent < 0.96)
			{
				LogiLcdColorSetText(2, wfps, 255, 126, 0);	//print to lcd as yellow
			}
			else if(fpspercent < 0.83)
			{
				LogiLcdColorSetText(2, wfps, 255, 0, 0);	//print to lcd as red
			}
			else
			{
				LogiLcdColorSetText(2, wfps, 255, 255, 255);	//print colour to lcd
			}
			LogiLcdColorSetText(3, wbit, 255, 255, 255);	//print colour to lcd
			//clear up
			delete [] fpsbit;
			delete [] wfps;
			delete [] wbit;
			fpsbyte.str(L"");
			sfps.str(L"");
			sbyte.str(L"");

			//dropped frames calculations
			frames << L"Dropped Frames: ";
			dropped = OBSGetFramesDropped();
			frames << dropped << L"(";
			total = OBSGetTotalFrames();
			percent = (double(dropped)/total)*100;
			frames << fixed << setprecision(2) << percent << L"%)";
			wchar_t *droppedframes = new wchar_t[frames.str().length() +1];
			wcscpy(droppedframes, frames.str().c_str());			//copy string

			//stream time
			stime << L"Stream Uptime: ";
			uptime = OBSGetTotalStreamTime()/1000;
			hour = (uptime / (60*60)) % 60;
			min = (uptime / 60) % 60;
			sec = (uptime % 60);
			
			stime << hour << L":";
			if(min < 10)
			{
				stime << L"0";
			}
			stime << min << L":";
			if(sec < 10)
			{
				stime << L"0";
			}
			stime << sec;

			wchar_t *streamtime = new wchar_t[stime.str().length() +1];
			wcscpy(streamtime, stime.str().c_str());			//copy string*/

			if(altdisplay)
			{
				//dropped frames
				LogiLcdMonoSetText(3, droppedframes);			//print to lcd
			}
			else
			{
				//stream time
				LogiLcdMonoSetText(3, streamtime);			//print to lcd
				
			}

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
				LogiLcdColorSetText(4, droppedframes, 225, 225, 225);	//print colour to lcd
			}
			LogiLcdColorSetText(5, streamtime, 255, 255, 255);	//print to lcd

			delete [] droppedframes;
			delete [] streamtime;
			frames.str(L"");
			stime.str(L"");
		}
		else
		{
			LogiLcdMonoSetBackground(mono_background_stopped);
			LogiLcdColorSetText(0, L"", 0, 0, 0);
			if(OBSGetMicMuted() && OBSGetDesktopMuted())
			{
				LogiLcdMonoSetText(0, L"OBS     Mute||Deaf       \u25CB");
				LogiLcdColorSetText(6, L"Muted and Deafened", 255, 0, 0);
			}
			else if(OBSGetMicMuted() && !OBSGetDesktopMuted())
			{
				LogiLcdMonoSetText(0, L"OBS     Mute|            \u25CB");
				LogiLcdColorSetText(6, L"Muted", 255, 126, 0);
			}
			else if(!OBSGetMicMuted() && OBSGetDesktopMuted())
			{
				LogiLcdMonoSetText(0, L"OBS          |Deaf       \u25CB");
				LogiLcdColorSetText(6, L"Deafened", 255, 126, 0);
			}
			else
			{
				LogiLcdMonoSetText(0, L"OBS                      \u25CB");
				LogiLcdColorSetText(6, L"", 0, 0, 0);
			}

			LogiLcdColorSetText(0, L"", 0, 0, 0);

			LogiLcdMonoSetText(2, L"FPS: --      Bitrate: ----");
			LogiLcdColorSetText(2, L"FPS: --");
			LogiLcdColorSetText(3, L"Bitrate: ----kb/s", 255, 255, 255);
			if(altdisplay)
			{
				LogiLcdMonoSetText(3, L"Dropped Frames: -(-.--%)");
			}
			else
			{
				LogiLcdMonoSetText(3, L"Stream Uptime: -:--:--");
			}
			LogiLcdColorSetText(4, L"Dropped Frames: -(-.--%)", 255, 255, 255);
			LogiLcdColorSetText(5, L"Stream Uptime: -:--:--", 255, 255, 255);
		}
		//update screen
		LogiLcdUpdate();

		Sleep(16);

		if(close.state())
		{
			done = true;
		}
	}
	LogiLcdShutdown();
	return NULL;
}
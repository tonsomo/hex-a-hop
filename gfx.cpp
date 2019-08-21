#include "state.h"

#ifdef _GP2X
#include <unistd.h>

enum MAP_KEY
{
	VK_UP         , // 0
	VK_UP_LEFT    , // 1
	VK_LEFT       , // 2
	VK_DOWN_LEFT  , // 3
	VK_DOWN       , // 4
	VK_DOWN_RIGHT , // 5
	VK_RIGHT      , // 6
	VK_UP_RIGHT   , // 7
	VK_START      , // 8
	VK_SELECT     , // 9
	VK_FL         , // 10
	VK_FR         , // 11
	VK_FA         , // 12
	VK_FB         , // 13
	VK_FY         , // 14
	VK_FX         , // 15
	VK_VOL_UP     , // 16
	VK_VOL_DOWN   , // 17
	VK_TAT          // 18
};
SDL_Joystick* joystick;
#endif

#ifdef WIN32
	#include <SDL_syswm.h>
	#include <shellapi.h> // Windows header for drag & drop
	#ifdef USE_BBTABLET
		#include "bbtablet/bbtablet.h"
	#endif
#else
	#undef USE_BBTABLET
#endif

StateMakerBase* StateMakerBase::first = 0;
State* StateMakerBase::current = 0;

#ifdef WIN32
	#include <windows.h>
	#include <winuser.h>
	#include <commdlg.h>
	#include <direct.h>

	bool tablet_system = false;

	char* LoadSaveDialog(bool save, bool levels, const char * title)
	{
		OPENFILENAME f;
		static char filename[1025] = "";
		static char path[1025] = "C:\\WINDOWS\\Desktop\\New Folder\\Foo\\Levels";
		char backupPath[1025];
		_getcwd(backupPath, sizeof(backupPath)/sizeof(backupPath[0])-1);
		
		memset(&f, 0, sizeof(f));

		#define FILTER(desc, f) desc " (" f ")\0" f "\0"
		f.lpstrFilter = FILTER("All known files","*.lev;*.sol")
						FILTER("Level files","*.lev")
						FILTER("Solution files","*.sol")
						FILTER("All files","*.*");
		#undef FILTER

		f.lStructSize = sizeof(f);
		f.lpstrFile = filename;
		f.nMaxFile = sizeof(filename);
		f.lpstrInitialDir = path;
		f.lpstrTitle = title;

		if (GetSaveFileName(&f)==TRUE)
		{
			// Remember user's choice of path!
			_getcwd(path, sizeof(path)/sizeof(path[0])-1);

			if (save)
			{
				int i = strlen(filename)-1;
				while (i>0 && filename[i]!='.' && filename[i]!='\\' && filename[i]!='/') i--;
				if (filename[i]!='.' && levels)
					strcat(filename, ".lev");
				if (filename[i]!='.' && !levels)
					strcat(filename, ".sol");
			}
			_chdir(backupPath);
			return filename;
		}

		_chdir(backupPath);
		return 0;
	}
#else
	char* LoadSaveDialog(bool save, bool levels, const char * title)
	{
		return 0;
	}
#endif

extern void test();

int mouse_buttons = 0;
int mousex= 10, mousey = 10;
#ifndef _GP2X
int noMouse = 0;
#else
int noMouse = 1;
#endif
int quitting = 0;

double stylusx= 0, stylusy= 0;
int stylusok= 0;
float styluspressure = 0;
SDL_Surface * screen = 0;
SDL_Surface * realScreen = 0;

extern State* MakeWorld();

bool fullscreen = false;

void Terminate()
{
	SDL_Quit();
#ifdef _GP2X
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
}

void InitScreen()
{
#ifdef USE_OPENGL
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

//	printf("SDL_SetVideoMode (OpenGL)\n");
	realScreen = SDL_SetVideoMode(
		SCREEN_W, SCREEN_H, // Width, Height
		0, // Current BPP
		SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0) );
#else
//	printf("SDL_SetVideoMode (non-OpenGL)\n");
	realScreen = SDL_SetVideoMode(
		SCREEN_W, SCREEN_H, // Width, Height
		0, // Current BPP
		SDL_SWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : 0) );
#endif

	if (screen)
		SDL_FreeSurface(screen);

	SDL_Surface* tempscreen = SDL_CreateRGBSurface(
		SDL_SWSURFACE, 
		SCREEN_W, SCREEN_H,
		16, 0xf800, 0x07e0, 0x001f, 0);

	screen = SDL_DisplayFormat(tempscreen);
	SDL_FreeSurface(tempscreen);
}

void ToggleFullscreen()
{
	#ifdef _GP2X
		fullscreen=true;
		return;
	#else
	fullscreen = !fullscreen;
	InitScreen();
	StateMakerBase::current->ScreenModeChanged();
	#endif
}

char base_path[1024] = "";

int main(int argc, char * argv[])
{
	atexit(Terminate);
	strcpy( base_path, argv[0] );
	base_path[strlen(base_path)-9] = '\0';
//	printf("SDL_Init\n");
	

/*	// Experimental - create a splash screen window whilst loading
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode( 200,200,0,SDL_NOFRAME );
	SDL_Rect r = {0,0,200,200};
	SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 0, 50));
	SDL_Flip(screen);
*/

	SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE);
	joystick=SDL_JoystickOpen(0);

	SDL_Surface* icon = SDL_LoadBMP("graphics/icon.bmp");
	if (icon)
	{
		static unsigned int mask[32] = {
			0x00001fc0,
			0x00003fe0,
			0x00007ff0,
			0x0f007df8,
			0x0000f0f8,
			0x0000f07c,
			0x0005f87c,
			0x0fbfff3c,

			0x1ffffffe,
			0x3ffffffe,
			0x3ffffffe,
			0x7ffffffe,
			0x7ffffffe,
			0x7ffffffe,
			0x7ffffffe,
			0xefffffff,

			0x1fffffff,
			0x3fffffff,
			0x3fffffff,
			0x3fffffff,
			0x3fffffff,
			0x3fffffff,
			0x3fffffff,
			0x3ffffffe,

			0x3ffffff8,
			0x3ffffff0,
			0x3ffffff0,
			0x3ffffff0,
			0x3fffffe0,
			0x3fffffe0,
			0x1ffffff0,
			0x1ffffff1,
		};
		for (int i=0; i<32; i++)
			mask[i] = mask[i]>>24 | (mask[i]>>8)&0xff00 | (mask[i]<<8)&0xff0000 | (mask[i]<<24)&0xff000000;
		SDL_WM_SetIcon(icon, (unsigned char*) mask);
		SDL_FreeSurface(icon);
	}

	InitScreen();

	SDL_WarpMouse(SCREEN_W/2, SCREEN_H/2);

#ifdef WIN32
	HWND hwnd = 0;
#endif
#ifdef USE_BBTABLET
	bbTabletDevice &td = bbTabletDevice::getInstance( );
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif

//	printf("Main loop...\n");
	
	StateMakerBase::GetNew();
	int time = SDL_GetTicks();

	while(!quitting)
	{
		SDL_Event e;
		while(!SDL_PollEvent(&e) && !quitting)
		{
			int x = SDL_GetTicks() - time;
			time += x;
			if (x<0) x = 0, time = SDL_GetTicks();
			if (x>500) x = 500;

			// experimental...
			if (!noMouse)
				StateMakerBase::current->Mouse(mousex, mousey, 0, 0, 0, 0, mouse_buttons);

			StateMakerBase::current->Update(x / 1000.0);
			StateMakerBase::current->Render();

			#ifdef USE_OPENGL
				SDL_GL_SwapBuffers();
			#else
				if (screen && realScreen!=screen)
				{
					SDL_Rect r = {0,0,SCREEN_W,SCREEN_H};
					SDL_BlitSurface(screen, &r, realScreen, &r);
				}
				SDL_Flip(realScreen);
			#endif

//#ifdef WIN32
//				Sleep(1);
//#endif
			SDL_Delay(10);

#ifdef USE_BBTABLET
			// Tablet ////////////////////////
			bbTabletEvent evt;
			while(hwnd!=NULL && td.getNextEvent(evt))
			{
				stylusok = 1;
				RECT r;
				if (tablet_system)
				{
					GetWindowRect(hwnd, &r);
					stylusx = evt.x * GetSystemMetrics(SM_CXSCREEN);
					stylusy = (1.0 - evt.y) * GetSystemMetrics(SM_CYSCREEN);
					stylusx -= (r.left + GetSystemMetrics(SM_CXFIXEDFRAME));
					stylusy -= (r.top + GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION));;
				}
				else
				{
					GetClientRect(hwnd, &r);
					stylusx = evt.x * r.right;
					stylusy = (1.0 - evt.y) * r.bottom;
				}
				styluspressure = (evt.buttons & 1) ? evt.pressure : 0;
 
				/*
				printf("id=%d csrtype=%d b=%x (%0.3f, %0.3f, %0.3f) p=%0.3f tp=%0.3f\n", 
					   evt.id,
					   evt.type,
					   evt.buttons,
					   evt.x,
					   evt.y,
					   evt.z,
					   evt.pressure,
					   evt.tpressure
					   );
				*/
			}

#endif
		}

		#ifdef _GP2X
		int nkey=0;
		int mod=0;
		#endif

		switch (e.type)
		{
/*			case SDL_VIDEOEXPOSE:
				w.Render();
				SDL_GL_SwapBuffers();
				break;*/
#ifdef WIN32
			case SDL_SYSWMEVENT:
			{
				SDL_SysWMmsg* m = e.syswm.msg;
				hwnd = m->hwnd;
				static bool init=false;
				if (!init)
				{
					init = true;
					DragAcceptFiles(hwnd, TRUE);
					#ifdef USE_BBTABLET
						td.initTablet(hwnd, tablet_system ? bbTabletDevice::SYSTEM_POINTER : bbTabletDevice::SEPARATE_POINTER );
						if (!td.isValid())
							 printf("No tablet/driver found\n");
					#endif
  				}
				if (m->msg == WM_DROPFILES)
				{
					HDROP h = (HDROP)m->wParam;
					
					char name[512];
					if (DragQueryFile(h, 0xffffffff, 0, 0) == 1)
					{
						DragQueryFile(h, 0, name, sizeof(name)/sizeof(name[0]));

						StateMakerBase::current->FileDrop(name);
					}

					DragFinish(h);
				}

				break;
			}
#endif

			case SDL_ACTIVEEVENT:
			{
				static int focus = 0;
				int gain = e.active.gain ? e.active.state : 0;
				int loss = e.active.gain ? 0 : e.active.state;
				focus = (focus | gain) & ~loss;
				if (gain & SDL_APPACTIVE)
					StateMakerBase::current->ScreenModeChanged();
				if (loss & SDL_APPMOUSEFOCUS)
					noMouse = 1;
				else if (gain & SDL_APPMOUSEFOCUS)
					noMouse = 0;
				break;
			}

			case SDL_MOUSEMOTION:
				noMouse = false;
				StateMakerBase::current->Mouse(e.motion.x, e.motion.y, e.motion.x-mousex, e.motion.y-mousey, 0, 0, mouse_buttons);
				mousex = e.motion.x; mousey = e.motion.y;
				break;
			case SDL_MOUSEBUTTONUP:
				noMouse = false;
				mouse_buttons &= ~(1<<(e.button.button-1));
				StateMakerBase::current->Mouse(e.button.x, e.button.y, e.button.x-mousex, e.button.y-mousey, 
										0, 1<<(e.button.button-1), mouse_buttons);
				mousex = e.button.x; mousey = e.button.y ;
				break;
			case SDL_MOUSEBUTTONDOWN:
				noMouse = false;
				mouse_buttons |= 1<<(e.button.button-1);
				StateMakerBase::current->Mouse(e.button.x, e.button.y, e.button.x-mousex, e.button.y-mousey, 
										1<<(e.button.button-1), 0, mouse_buttons);
				mousex = e.button.x; mousey = e.button.y ;
				break;

			case SDL_KEYUP:
				StateMakerBase::current->KeyReleased(e.key.keysym.sym);
				break;
			#ifdef _GP2X
			case SDL_JOYBUTTONUP:
				if(e.jbutton.button==VK_UP) nkey=SDLK_UP;
				if(e.jbutton.button==VK_DOWN) nkey=SDLK_DOWN;
				if(e.jbutton.button==VK_LEFT) nkey=SDLK_LEFT;
				if(e.jbutton.button==VK_RIGHT) nkey=SDLK_RIGHT;
				if(e.jbutton.button==VK_START) nkey=SDLK_ESCAPE;
				if(e.jbutton.button==VK_FX) nkey=SDLK_RETURN;
				if(e.jbutton.button==VK_UP_LEFT) nkey=SDLK_q;
				if(e.jbutton.button==VK_UP_RIGHT) nkey=SDLK_e;
				if(e.jbutton.button==VK_DOWN_LEFT) nkey=SDLK_a;
				if(e.jbutton.button==VK_DOWN_RIGHT) nkey=SDLK_d;
				if(e.jbutton.button==VK_FY) nkey=SDLK_z;
				StateMakerBase::current->KeyReleased(nkey);
				break;
			case SDL_JOYBUTTONDOWN:
				if(e.jbutton.button==VK_UP) nkey=SDLK_UP;
				if(e.jbutton.button==VK_DOWN) nkey=SDLK_DOWN;
				if(e.jbutton.button==VK_LEFT) nkey=SDLK_LEFT;
				if(e.jbutton.button==VK_RIGHT) nkey=SDLK_RIGHT;
				if(e.jbutton.button==VK_START) nkey=SDLK_ESCAPE;
				if(e.jbutton.button==VK_FX) nkey=SDLK_RETURN;
				if(e.jbutton.button==VK_UP_LEFT) nkey=SDLK_q;
				if(e.jbutton.button==VK_UP_RIGHT) nkey=SDLK_e;
				if(e.jbutton.button==VK_DOWN_LEFT) nkey=SDLK_a;
				if(e.jbutton.button==VK_DOWN_RIGHT) nkey=SDLK_d;
				if(e.jbutton.button==VK_FY) nkey=SDLK_z;
				StateMakerBase::current->KeyPressed(nkey,mod);
				break;
			#endif
			case SDL_KEYDOWN:
			{
				SDL_KeyboardEvent & k = e.key;

				if (k.keysym.sym==SDLK_F4 && (k.keysym.mod & KMOD_ALT))
				{
					quitting = 1;
				}
				else if (k.keysym.sym==SDLK_F12)	
				{
					// Toggle system pointer controlled by tablet or not
					#ifdef USE_BBTABLET
						if (td.isValid())
						{
							tablet_system = !tablet_system;
							td.setPointerMode(tablet_system ? bbTabletDevice::SYSTEM_POINTER : bbTabletDevice::SEPARATE_POINTER);
						}
					#endif
				}
				else if (k.keysym.sym==SDLK_RETURN && (k.keysym.mod & KMOD_ALT) && !(k.keysym.mod & KMOD_CTRL))
				{
					ToggleFullscreen();
				}
				else if (StateMakerBase::current->KeyPressed(k.keysym.sym, k.keysym.mod))
				{
				}
				else if ((k.keysym.mod & (KMOD_ALT | KMOD_CTRL))==0)
				{
					StateMakerBase::GetNew(k.keysym.sym);
				}
			}
			break;

			case SDL_QUIT:
				quitting = 1;
				break;
		}
	}

	SDL_Quit();
	return 0;
}

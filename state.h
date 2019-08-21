
//
// Config block
//


// Uncomment this to check cross-platform compilation compatibility
// #undef WIN32

//#define USE_BBTABLET
//#define USE_OPENGL

#define SCREEN_W 640
#define SCREEN_H 480

//
// End of config block
//

// Hacky workaround for MSVC's broken for scoping
#define for if (0) ; else for

// LINUX: SDL/
#include <SDL/SDL.h>
#ifdef USE_OPENGL
#include <SDL/SDL_OpenGL.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern SDL_Surface * screen;

// LINUX: WHAT ???
#ifdef WIN32
#define FATAL() do{__asm{int 3};}while(0)
#else
//#define FATAL() {printf("Fatal error"); exit(0);    }
static inline void FATAL () { fprintf(stderr, "Fatal error\n"); exit(0); }
#endif


class State
{
public:
	virtual ~State() {}

	virtual bool KeyPressed(int key, int mod) = 0;
	virtual void KeyReleased(int key) {};
	virtual void Mouse(int x, int y, int dx, int dy, int buttons_pressed, int buttons_released, int buttons) = 0;
	virtual void Update(double timedelta) = 0;
	virtual void Render() = 0;
	virtual void FileDrop(const char* filename) = 0;
	virtual void ScreenModeChanged() {};
};

/************************************************************************
// TEMPLATE - copy & paste

#define ClassName NEWSTATE
class ClassName : public State
{
public:
	virtual bool KeyPressed(int key, int mod) 
	{ 
		return false; 
	}
	virtual void KeyReleased(int key) 
	{ 
	}
	virtual void Mouse(int x, int y, int dx, int dy, int buttons_pressed, int buttons_released, int buttons) 
	{
	}
	virtual void FileDrop(const char* filename) 
	{
	}
	virtual void Update(double timedelta)
	{
		// TODO
	}
	virtual void Render()
	{
		// TODO
	}
	virtual void ScreenModeChanged() 
	{
		// TODO
	}
};
MAKE_STATE(ClassName, _KEY_, false);

************************************************************************/

class StateMakerBase
{
	static StateMakerBase* first;
	StateMakerBase* next;
	int key;
	bool start;
protected:
	State* state;
public:
	static State* current;

public:
	StateMakerBase(int key, bool start) : state(NULL)
	{
		for (StateMakerBase* s = first; s; s=s->next)
			if(key == s->key)
			{
				FATAL();
				return;
			}
		this->key = key;
		this->start = start;
		next = first;
		first = this;
	}
	virtual State* Create() = 0;
	void Destroy()
	{
		delete state;
		state = 0;
	}
	static State* GetNew(int k)
	{
		for (StateMakerBase* s = first; s; s=s->next)
			if(k==s->key)
				return current = s->Create();
		return current;
	}
	static State* GetNew()
	{
		if (!first)
		{
			FATAL();
			return 0;
		}
		for (StateMakerBase* s = first; s; s=s->next)
			if(s->start)
				return current = s->Create();
		return current = first->Create();
	}
	static void DestroyAll()
	{
		for (StateMakerBase* s = first; s; s=s->next)
			s->Destroy();
		current = 0;
	}
};

template<class X>
class StateMaker : public StateMakerBase
{
public:
	StateMaker(int key, bool start=false) : StateMakerBase(key, start)
	{}
	State* Create() 
	{ 
		if (!state) state = new X;
		return state;
	}
};

#define MAKE_STATE(x,key,start) static StateMaker<x> _maker_##x(key, start);

extern int mouse_buttons, mousex, mousey, noMouse;
extern double stylusx, stylusy;
extern float styluspressure;
extern int stylusok;
extern int quitting;

char* LoadSaveDialog(bool save, bool levels, const char * title);

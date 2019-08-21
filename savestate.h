
struct HexPuzzle;

class LevelSave
{
	friend struct HexPuzzle;

	char * bestSolution;
	int bestSolutionLength;
	int bestScore;
	#define NUM_LAST_SCORES 19
	int lastScores[NUM_LAST_SCORES];
	int unlocked;
public:
	LevelSave()
	{
		Clear();
	}
	void Clear()
	{
		unlocked = 0;
		bestSolution = 0;
		bestScore = 0;
		bestSolutionLength = 0;
		memset(lastScores, 0, sizeof(lastScores));	
	}
	void LoadSave(FILE* f, bool save)
	{
		typedef unsigned int _fn(void*, unsigned int, unsigned int, FILE*);
		_fn * fn = save ? (_fn*)fwrite : (_fn*)fread;

		fn(&bestSolutionLength, sizeof(bestSolutionLength), 1, f);
		fn(&bestScore, sizeof(bestScore), 1, f);
		fn(&lastScores, sizeof(lastScores), 1, f);
		fn(&unlocked, sizeof(unlocked), 1, f);		
		
		if (bestSolutionLength)
		{
			if (!save) SetSolution(bestSolutionLength);
			fn(bestSolution, sizeof(bestSolution[0]), bestSolutionLength, f);
		}
	}

	void Dump()
	{
		for (int j=1; j<NUM_LAST_SCORES; j++)
			if (lastScores[j]==lastScores[0])
				lastScores[j] = 0;

/*		for (int i=0; i<NUM_LAST_SCORES && lastScores[i]; i++)
			if (lastScores[i] != bestScore)
				printf("\t% 8d\n", lastScores[i]);*/
	}
	bool Completed()
	{
		return bestScore != 0;
	}
	bool IsNewCompletionBetter(int score)
	{
		for (int i=0; i<NUM_LAST_SCORES; i++)
		{
			if (lastScores[i]==0)
				lastScores[i] = score;
			if (lastScores[i]==score)
				break;
		}

		if (!Completed())
			return true;

		return score <= bestScore;
	}
	bool PassesPar(int par)
	{
		if (!Completed())
			return false;
		return bestScore <= par;
	}
	int GetScore()
	{
		return bestScore;
	}
	void SetScore(int s)
	{
		bestScore = s;
	}
	void SetSolution(int l) { 
		delete [] bestSolution;
		bestSolutionLength = l;
		bestSolution = new char [ l ];
	}
	void SetSolutionStep(int pos, int val)
	{
		bestSolution[pos] = val;
	}
};

class SaveState
{
	struct X : public LevelSave
	{
		X* next;
		char* name;

		X(const char* n, X* nx=0) : next(nx)
		{
			name = new char[strlen(n)+1];
			strcpy(name, n);
		}
		~X()
		{
			delete [] name;
		}
	};

	struct General {
		int scoringOn;
		int hintFlags;
		int completionPercentage;
		int endSequence;
		int masteredPercentage;
		int pad[6];
	};

	X* first;

	void ClearRaw()
	{
		memset(&general, 0, sizeof(general));
		general.hintFlags = 1<<31 | 1<<30;

		X* x=first;
		while (x)
		{
			X* nx = x->next;
			delete x;
			x = nx;
		}
		first = 0;

	}

public:
	
	General general;


	SaveState() : first(0)
	{
		Clear();
	}

	~SaveState()
	{
		ClearRaw();
	}

	void Clear()
	{
		ClearRaw();
		ApplyStuff();
	}

	void GetStuff();
	void ApplyStuff();

	void LoadSave(FILE* f, bool save)
	{
		if (save)
		{
			GetStuff();

			//printf("----\n");

			fputc('2', f);
			fwrite(&general, sizeof(general), 1, f);
			for(X* x=first; x; x=x->next)
			{
				short len = strlen(x->name);
				fwrite(&len, sizeof(len), 1, f);
				fwrite(x->name, len, 1, f);

				x->LoadSave(f,save);

				if (x->Completed())
				{
					//printf("% 8d %s\n", x->GetScore(), x->name);
					x->Dump();
				}
			}
		}
		else
		{
			ClearRaw();
			int v = fgetc(f);
			if (v=='2')
			{
				fread(&general, sizeof(general), 1, f);
				v = '1';
			}
			if (v=='1')
			{
				while(!feof(f))
				{
					char temp[1000];
					short len;
					fread(&len, sizeof(len), 1, f);
					if (feof(f)) break;
					fread(temp, len, 1, f);
					temp[len] = 0;
					first = new X(temp, first);

					first->LoadSave(f,save);
				}
			}

			ApplyStuff();
		}
	}

	LevelSave* GetLevel(const char * name, bool create)
	{
		char * l = strstr(name, "Levels");
		if (l)
			name = l;

		X* x = first;
		if (x && strcmp(name, x->name)==0) return x;
		while (x && x->next)
		{
			if (strcmp(name, x->next->name)==0) return x->next;
			x = x->next;
		}
		if (create)
		{
			X* n = new X(name);
			if (x)
				x->next = n;
			else
				first = n;
			return n;
		}
		else
		{
			return 0;
		}
	}
};

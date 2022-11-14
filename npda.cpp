//Name: 		Ryan DeSellems
//Date: 		11/11/2022
//Course: 		COMP 4400
//Prof:			Hossain
//File Desc:	A program that will use a pushdown automata defined in a file to determine 
//				if a password meets a given language, using recursive processing

#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
using namespace std;

string  getPassword();

void    startProcess(string pword, unordered_map<string,string> &npdaToUse);

bool    isFinalState(string currentState, unordered_map<string,string> &npdaToUse);

bool 	process(string password, string currentState, 
				string stackTop, unordered_map<string,string> &npdaToUse);


void    buildAutomata(unordered_map<string,string> &npdaToBuild);

string modifyStack(string stackToChange, string stackTop);

string getKey(string passwordIn, string stateIn, string stackIn, bool lambdaToggle);

string getMapResult(string key, unordered_map<string,string> &npda);

string getNewState(string keyResult);

string getNewStackTop(string keyResult);

//=============================================================================
int main()
{
	unordered_map<string, string> passAutomata;

	buildAutomata(passAutomata);            										// build to dfa from transition func.
	cout << "Auto build.\n";
	while(true)
	{
		string w = getPassword();           										// w is user string to process
		startProcess(w,passAutomata);
	}
}
//=============================================================================
string getPassword()
{
	string input;                           										// get password
	cout << "Enter password string [Ctrl + C to Exit]: ";
	getline(cin, input);
	return input;
}
//=============================================================================
void startProcess(string password, unordered_map<string,string> &npdaToUse)
{
	string initialState = "q0";
	string stackStart  = "z";

	if(process(password,initialState,stackStart,npdaToUse))							// when function returns, it will display proper accepteance
	{
		cout << "Accepted.\n";
	}
	else
	{
		cout << "Rejected.\n";
	}
}
//=============================================================================
bool process(string password, string currentState, 
			 string stack, unordered_map<string,string> &npda)
{
	if(password == "")
	{
		password = "*";																// if the password is empty, symbolized lambda
	}

	if(password == "*" && isFinalState(currentState,npda))
	{
		cout << "|- (" + currentState + ',' + password + ',' + stack + ")" << endl; // display instantaneous description
		return true;
	}

	string charKey 	  	  = getKey(password,currentState,stack,false);				// get key to search map for
	string lambdaKey 	  = getKey(password,currentState,stack,true);				//...
	string charResult 	  = getMapResult(charKey,npda);								// get result from map if exists
	string lambdaResult   = getMapResult(lambdaKey,npda);							//...

	if(charResult.compare("NO_KEY") == 0 && lambdaResult.compare("NO_KEY") == 0)
	{
		return false;																// key not located in map, return
	}

	string charState  	  = getNewState(charResult);								// break up the result from the map
	string charStackTop   = getNewStackTop(charResult);								// into proper components
	string lambdaState    = getNewState(lambdaResult);
	string lambdaStackTop = getNewStackTop(lambdaResult);
	
	string cStack = modifyStack(stack,charStackTop);								// modify stack determines whether to push or pop
	string lStack = modifyStack(stack,lambdaStackTop);								// given input

	if(currentState == "q0" && stack[0] == 'z' )
	{
		cout << "(" + currentState + ',' + password + ',' + stack + ")" << endl;	// display initial state description
	}
	else
	{
		cout << "|- (" + currentState + ',' + password + ',' + stack + ")" << endl; // display processing description
	}

	if(process(password.substr(1),charState,cStack,npda) || process(password,lambdaState,lStack,npda))
	{
		return true;																// if either the input symbol path
	}																			    // or the path using lambda for the input 
	return false;																	// is true, return true, false otherwise
}
//=============================================================================
bool isFinalState(string currentState, unordered_map<string,string> &npdaToUse)
{
	auto resVal = npdaToUse.find(currentState);										// just look up value of state
	return (resVal -> second == "accept");
}
//=============================================================================
string modifyStack(string stackToChange, string stackTop)
{
	stackToChange = stackToChange.substr(1);										// pop first char of stack

	if(stackTop == "*")
	{
		return stackToChange;														// done if lambda
	}	
	else
	{
		return stackTop + stackToChange;											// push new char if not lambda
	}
}
//=============================================================================
string getKey(string passwordIn, string stateIn, string stackIn, bool lambdaToggle)
{
	if(passwordIn == "")															// build lookup key for function params.
	{
		lambdaToggle = true;														// if string is empty, lambda will be used
	}
	char firstChar = '*';														    // assume lambda by default
	if(!lambdaToggle)																// if lambda not needed, use first password symbol
	{																				// in the key
		firstChar = passwordIn[0];	
	}
	string rs;
	rs = stateIn + "." + firstChar + "." + stackIn[0];							 	// format key and return, not done on fly for easy debugging
	return rs;
}
//=============================================================================
string getMapResult(string key, unordered_map<string,string> &npda)
{
	auto result = npda.find(key);													// look up key formed in above function

	if(result != npda.end())														// if it exists, return it
	{
		return result->second;
	}
	else																			// if it doesn't, let caller know to avoid crash
	{
		return "NO_KEY";
	}
}
//=============================================================================
string getNewState(string keyResult)
{																					//State is first in key pair form (state.stackTop)
	return keyResult.substr(0,keyResult.find('.'));									// take result from above and  get the state
}
//=============================================================================
string getNewStackTop(string keyResult)
{																					// Stack top is the second in key pair form (state.stackTop)
	return keyResult.substr(keyResult.find('.') + 1);								// take result above and get the stack top
}
//=============================================================================
void buildAutomata(unordered_map<string,string> &npdaToBuild)
{
	string   transitionRule;                                						//string that will be used to hold transition function
	ifstream inFile;
	string 	 firstKey;                                        						//state and input symbol to process
	string   secondKey;                                       						//state to map to firstKey
	int    	 equalPos;                                        						//location of equal sign in transition function

	inFile.open("npdapasswordtransition.txt");                  					//open file containing transition function
	if(!inFile.is_open())                                   						//handle file opening
	{
		cout << "Error loading transition function file.\nProcess terminated.\n";
		exit(0);
	}

	while(getline(inFile, transitionRule))                   						//while not eof
	{
		if(transitionRule.find("q") == 0)                    						//make sure line starts with transition rule
		{
			equalPos  = transitionRule.find('=');            						//get position of equal sign, which divides the rule
			firstKey  = transitionRule.substr(0,equalPos);   						//firstKey is on left side of equal sign
			secondKey = transitionRule.substr(equalPos+1);   						//secondKey is on right side
			npdaToBuild[firstKey] = secondKey;               						//insert key into dfa map
		}
	}
}
//=============================================================================
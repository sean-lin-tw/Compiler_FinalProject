#define DEBUG 0
#define KEYWORD 	0
#define OPERATOR	1
#define SPECIAL		2
#define IDENTIFIER	3
#define NUMBER		4
#define CHARACTER	5
#define ERROR		6

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
   
using namespace std;
/* data structure for non-terminals */
class non_terminal
{
	public:
		string name;
		bool nullable;
		set<string> First;	//use set
		set<string> Follow;	//use set
		non_terminal(string str)
		{
			name = str;
		}
	private:	
};
/* tree structure */
class node
{
	public:
		string name;
		vector<node*> childnode;
		node(string str)
		{
			name = str;
		}
	private:
};
class TOKEN
{
	public:
		string name;
		string value;
		TOKEN(string NAME, string VALUE)
		{
			name = NAME;
			if(NAME == "id" || NAME == "num")
				value = VALUE;			
		}	
	private:
};

/* functions */
int findType(string token);
int checkType(string token);
bool isID(string token);
bool isNum(string token);
bool isChar(string token);

bool getNullable(string head);
bool hasEpsilon(string head);
bool hasTerminal(vector<vector<string> >::iterator rule);

void getFirst(non_terminal* NT_array[]);
bool addFirst(string head, string body_symbol, non_terminal* NT_array[]);
bool isNullable(string variable, non_terminal* NT_array[]);
bool unionFirst(int num1, int num2, non_terminal* NT_array[]);

void getFollow(non_terminal* NT_array[]);
bool addFollow1(string head, string body_symbol, non_terminal* NT_array[]);
bool addFollow2(string front, string back, non_terminal* NT_array[]);
bool unionFollow1(int num1, int num2, non_terminal* NT_array[]);
bool unionFollow2(int num1, int num2, non_terminal* NT_array[]);

bool isTerminal(string symbol);
int findNTnum(string symbol, non_terminal* NT_array[]);

string findToken(string label,string lexeme);
void buildParseTree(node* current);
void addNode1(node* current, vector<string>::iterator it1, vector<string>::iterator it2);
void addNode2(node* current, string value);
vector<string>::iterator findProdStart(string non_terminal, string terminal);
vector<string>::iterator findProdEnd(string non_terminal, string terminal);

void setSymbolTable(void);

/* global variables */
map < string,vector< vector<string> > > production;
map<string, map<string, vector<string> > >LLtable;//LL table structure
vector<string> NT_namelist;
int NT_num = 0;
vector<TOKEN> tokenlist;
vector<TOKEN>::iterator reader;

int main(int argc, char *argv[])
{
	/* lexical analysis */
	ifstream program("main.c"); 
	ofstream tokenFile("token.txt");
	/* extract a line from the program */
	char buff[100];
	int i=1;	//used for couting
	while(program.getline(buff,100))
	{
		tokenFile << "Line  " << i << ":" <<endl;
		if(buff[0] == '/' && buff[1] == '/')
			continue;	// Neglect the comments
		else	
		{
			string token;
			string line = buff;
			istringstream stream(line);
			while(stream >> token)
			{
				string type;
				switch(findType(token))
				{
					case KEYWORD:
						type = "<Keyword>";
						break;
					case OPERATOR:
						type = "<Operator>";
						break;
					case SPECIAL:
						type = "<Special>";
						break;
					case IDENTIFIER:
						type = "<Identifier>";
						break;
					case NUMBER:
						type = "<Number>";
						break;
					case CHARACTER:
						type = "<Char>";
						break;
					case ERROR:
						type = "<Error>";
						break;						
				}
				/* Write to token.txt */
				tokenFile << "\t\t" << type << "\t: " << token << endl;
			}
			i++;
		}
	}	
	program.close();
	tokenFile.close();

	/* parsing */
	ifstream grammar("grammar.txt"); 	
	vector< vector<string> > bodylist_all;	// production bodies of all rules of a variable
	
	string head, body, temp1;

	/* establish production rules */ 
	while(grammar.getline(buff,100))
	{			
		vector<string> bodylist;				// production body of a rule

		bool ready;			/* Finish dealing with all the productions of a variable */
		
		if(buff[0] != ' ')	/* Add to the key of the map */
		{
			temp1 = buff;			
			ready = true;
		}	
		else				/* Add to the value of the map */
		{
			body = buff;
			
			string temp2;
			istringstream stream(buff);
			while(stream >> temp2)
			{
				bodylist.push_back(temp2);				
			}
			bodylist_all.push_back(bodylist);
			ready = false;
		}
		bodylist.clear();
		
		if((ready && (!body.empty())) || grammar.eof())
		{
			production.insert( pair< string, vector< vector<string> > >(head,bodylist_all) );
			NT_namelist.push_back(head);
			NT_num++;					
			bodylist_all.clear();
		}
		head = temp1;
	}	
	
	/* nullable, first, follow */
	/* create data strucures */	
	non_terminal* NT_array[NT_num];
	i = 0;
	for(map< string,vector<vector<string> > >::iterator it = production.begin(); it != production.end(); it++)
	{
		NT_array[i++] = new non_terminal((*it).first);	//array of objects
	}	

	/* nullable */
	for(int i=0; i<NT_num; i++)
	{
		NT_array[i]->nullable = getNullable(NT_array[i]->name);			
	}	
	/* first */
	getFirst(NT_array);
	
	/* follow */
	getFollow(NT_array);
	
	#if DEBUG
	/* test area */	
	for(int i=0; i<NT_num; i++)
	{
		if(NT_array[i]->nullable)
			cout<<NT_array[i]->name<<"\tO"<<endl;
		else
			cout<<NT_array[i]->name<<"\tX"<<endl;
	}
	for(int i=0; i<NT_num; i++)
	{
		cout<<NT_array[i]->name<<"  ";
		for(set<string>::iterator it = NT_array[i]->First.begin(); it != NT_array[i]->First.end(); it++)
			cout<<*it<<" ";
		cout<<endl;	
	}
	for(int i=0; i<NT_num; i++)
	{
		cout<<NT_array[i]->name<<"  ";
		for(set<string>::iterator it = NT_array[i]->Follow.begin(); it != NT_array[i]->Follow.end(); it++)
			cout<<*it<<" ";
		cout<<endl;	
	}
	#endif		
		 	
	
	/* Print set to file */
	ofstream setFile("set.txt");
	setFile<<"Nullable"<<endl;
	for(int i=0; i<NT_num; i++)
	{
		string null_str = (NT_array[i]->nullable) ?"true" :"false";
		int length = (NT_array[i]->name).size();
		
		if(length < 8)
			setFile<<NT_array[i]->name<<"\t\t\t: "<<null_str<<endl;
		else if(length >= 8 && length < 14)
			setFile<<NT_array[i]->name<<"\t\t: "<<null_str<<endl;
		else
			setFile<<NT_array[i]->name<<"\t: "<<null_str<<endl;
	}	
	setFile<<"\nFirst"<<endl;
	for(int i=0; i<NT_num; i++)
	{
		int length = (NT_array[i]->name).size();
		if(length < 8)
		{
			setFile<<NT_array[i]->name<<"\t\t\t: ";
			for(set<string>::iterator it = NT_array[i]->First.begin(); it != NT_array[i]->First.end(); it++)
				setFile<<*it<<" ";
			setFile<<endl;	
		}
		else if(length >= 8 && length < 14)
		{
			setFile<<NT_array[i]->name<<"\t\t: ";
			for(set<string>::iterator it = NT_array[i]->First.begin(); it != NT_array[i]->First.end(); it++)
				setFile<<*it<<" ";
			setFile<<endl;	
		}
		else
		{
			setFile<<NT_array[i]->name<<"\t: ";
			for(set<string>::iterator it = NT_array[i]->First.begin(); it != NT_array[i]->First.end(); it++)
				setFile<<*it<<" ";
			setFile<<endl;	
		}
	}	
	setFile<<"\nFollow"<<endl;
	for(int i=0; i<NT_num; i++)
	{
		int length = (NT_array[i]->name).size();
		if(length < 8)
		{
			setFile<<NT_array[i]->name<<"\t\t\t: ";
			for(set<string>::iterator it = NT_array[i]->Follow.begin(); it != NT_array[i]->Follow.end(); it++)
				setFile<<*it<<" ";
			setFile<<endl;	
		}
		else if(length >= 8 && length < 14)
		{
			setFile<<NT_array[i]->name<<"\t\t: ";
			for(set<string>::iterator it = NT_array[i]->Follow.begin(); it != NT_array[i]->Follow.end(); it++)
				setFile<<*it<<" ";
			setFile<<endl;	
		}
		else
		{
			setFile<<NT_array[i]->name<<"\t: ";
			for(set<string>::iterator it = NT_array[i]->Follow.begin(); it != NT_array[i]->Follow.end(); it++)
				setFile<<*it<<" ";
			setFile<<endl;	
		}
	}
	setFile.close();
	
	/* Parse Table */
	ofstream LLtableFile("LLtable.txt");
	
	for(map< string,vector<vector<string> > >::iterator it1 = production.begin(); it1 != production.end(); it1++)
	{	
		map<string, vector<string> >tableRow;
		for(vector<vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
		{				
			set<string> proFirst;
			set<string> proFollow;
			for(vector<string>::iterator it3=it2->begin(); it3 != it2->end(); it3++)
			{
				if(*it3 == "epsilon")	//for epsilon-production
				{
					int j = findNTnum(it1->first,NT_array);
					proFollow.insert(NT_array[j]->Follow.begin(),NT_array[j]->Follow.end());
					
					for(set<string>::iterator it5 = proFollow.begin(); it5 != proFollow.end(); it5++)
					{
						vector<string>tempRule;
						tempRule.push_back("epsilon");
						tableRow.insert(pair<string, vector<string> >(*it5,tempRule));					
					}
				}
				else	//First set of the production
				{
					if( isTerminal(*it3) )
						proFirst.insert(*it3);
					else
					{
						int i = findNTnum(*it3,NT_array);
						proFirst.insert(NT_array[i]->First.begin(),NT_array[i]->First.end());
						if(isNullable(*it3,NT_array))
							continue;
					}				
					for(set<string>::iterator it4 = proFirst.begin(); it4 != proFirst.end(); it4++)
					{
						tableRow.insert(pair<string, vector<string> >(*it4,*it2));					
					}
				}							
				break;	//next rule				
			}	
		}
		LLtable.insert( pair<string, map<string, vector<string> > >(it1->first,tableRow) );
		tableRow.clear();
	}
	
	LLtableFile<<"S"<<endl;
	for(map<string, map<string, vector<string> > >::iterator it1 = LLtable.begin(); it1 != LLtable.end(); it1++)
	{
		for(map<string, vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
		{
			int length = (it1->first).size();
			
			if(length < 8)
				LLtableFile<<it1->first<<"\t\t\t"<<it2->first<<"\t\t";
			else if(length >= 8 && length < 14)
				LLtableFile<<it1->first<<"\t\t"<<it2->first<<"\t\t";
			else
				LLtableFile<<it1->first<<"\t"<<it2->first<<"\t\t";
				
			for(vector<string>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); it3++)
			{				
				LLtableFile<<*it3<<" ";	
			}
			LLtableFile<<endl;
		}
	}
	LLtableFile.close();
	
	/* Parse Tree */	
	ofstream parseTree("tree.txt");
	ifstream tokenFile2("token.txt");
	
	tokenFile2.getline(buff,100);
	while(tokenFile2.getline(buff,100))
	{
		istringstream stream(buff);
		string temp, label, lexeme, token, value;
		int count = 0;
		if(buff[0] == 'L')
			continue;
		else
		{		
			while(stream >> temp)
			{
				if(count == 0)
					label = temp;
				else if(count == 2)
					lexeme = temp;
				count++;	
			}
			token = findToken(label,lexeme);
			
			if((token == "id") || (token == "num"))
				value = lexeme;
			else	
				value = "NONE";
			TOKEN* tk = new TOKEN(token,value);
			tokenlist.push_back(*tk);
		}
	}
	TOKEN* tk = new TOKEN("$","NONE");
	tokenlist.push_back(*tk);
	
	//for(vector<TOKEN>::iterator it = tokenlist.begin(); it != tokenlist.end(); it++)
	//	cout<<it->name<<endl;
		
	
	node* root = new node("S");
	reader = tokenlist.begin();
	//buildParseTree(root);
	
	#if DEBUG
	map<string, map<string, vector<string> > >::iterator it1 = LLtable.find(non_terminal)
	map<string, vector<string> >::iterator it2 = it1->second.find(terminal)
	vector<string>::iterator it3 = it2->second;
	
	
	for(map<string, map<string, vector<string> > >::iterator it1 = LLtable.begin(); it1 != LLtable.end(); it1++)
	{
		for(map<string, vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
		{
			for(vector<string>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); it3++)
			{
				
				
				.find()
			}
		}	
	}	
	
	
	/* Semantic Tree */
	#endif
	/* Symbol Table */
	setSymbolTable();
	#if DEBUG
	/* Quadruples */
	
	/* Machine Code */
	
	#endif
	//system("pause");
	return 0;

}
/* Find out types of tokens */
int findType(string token)
{
	int type;
	
	/* Keywords */
	if(token == "int")
		type = KEYWORD;
	else if(token == "char")
		type = KEYWORD;
	else if(token == "return")
		type = KEYWORD;
	else if(token == "if")
		type = KEYWORD;
	else if(token == "else")
		type = KEYWORD;
	else if(token == "while")
		type = KEYWORD;
	/* Operators */
	else if(token == "=")
		type = OPERATOR;
	else if(token == "!")
		type = OPERATOR;
	else if(token == "+")
		type = OPERATOR;
	else if(token == "-")
		type = OPERATOR;
	else if(token == "*")
		type = OPERATOR;
	else if(token == "/")
		type = OPERATOR;
	else if(token == "==")
		type = OPERATOR;
	else if(token == "!=")
		type = OPERATOR;
	else if(token == "<")
		type = OPERATOR;		
	else if(token == ">")
		type = OPERATOR;
	else if(token == "<=")
		type = OPERATOR;
	else if(token == ">=")
		type = OPERATOR;
	else if(token == "&&")
		type = OPERATOR;
	else if(token == "||")
		type = OPERATOR;			
	/* Special Symbols */
	else if(token == "[")
		type = SPECIAL;
	else if(token == "]")
		type = SPECIAL;
	else if(token == "(")
		type = SPECIAL;
	else if(token == ")")
		type = SPECIAL;
	else if(token == "{")
		type = SPECIAL;	
	else if(token == "}")
		type = SPECIAL;
	else if(token == ";")
		type = SPECIAL;
	else if(token == ",")
		type = SPECIAL;
	/* Identifiers, Numbers, Chars */
	else
		type = checkType(token);

	return type;
}
/* Use regular expression*/
int checkType(string token)
{
	if( isID(token) )
		return IDENTIFIER;
	else if( isNum(token) )
		return NUMBER;
	else if( isChar(token) )
		return CHARACTER;
	else 
		return ERROR;
}
//[a-zA-Z_][a-zA-Z0-9_]*		
bool isID(string token)
{
	bool ID = true;
	string::iterator it = token.begin();
	char ch = (int)(*it);
	
	if(ch>=65 && ch<=90){}			//A-Z
	else if(ch>=97 && ch<=122){}	//a-z
	else if(ch==95){}				//_		
	else
		return false;
		
	for (it=token.begin()+1; it!=token.end(); ++it)
	{
		ch = (int)(*it);

		if(ch>=65 && ch<=90){}			//A-Z
		else if(ch>=97 && ch<=122){}	//a-z
		else if(ch>=48 && ch<=57){}		//0-9
		else if(ch==95){}				//_
		else
			ID = false;
		if(!ID)
			break;
	}
	return ID;
}
//[0-9]+
bool isNum(string token)
{
	bool num = true;
	for (string::iterator it=token.begin(); it!=token.end(); ++it)
	{
		switch(*it)
		{
			case '0': break;
			case '1': break;
			case '2': break;
			case '3': break;
			case '4': break;
			case '5': break;
			case '6': break;
			case '7': break;
			case '8': break;
			case '9': break;
			default : num = false;	break;
		}	
		if(!num)
			break;
	}
	return num;
}

bool isChar(string token)
{
	string::iterator begin = token.begin();
	string::iterator end = token.end()-1;
	if(*begin == '\'' && *end == '\'')
		return true;
	else 
		return false;
}

bool getNullable(string head)
{
	if(hasEpsilon(head))	//1.對於每個non-terminal,先看能不能產出epsilon,如果有就直接結束
		return true;
	else
	{
		map< string,vector<vector<string> > >::iterator it1 = production.find(head); 
		for(vector< vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
		{
			if(hasTerminal(it2))	//2.如果沒有epsilon,就要把每條rule跑過一遍,對於每條rule,先看有沒有terminal,如果有就直接跳過,如果全部被跳過就是un-nullable
				continue;			//3.如果還是沒有terminal的話,就從body的第一個non-terminal開始,去檢查它的production rule
			else
			{
				bool null = true;
				for(vector<string>::iterator it3=it2->begin(); it3 != it2->end(); it3++)
				{
					if(getNullable(*it3))
						continue;
					else
					{
						null = false;
						break;
					}	
				}
				if(null)
					return true;
			}	
		}
		
		
		return false;
	}		
}

bool hasEpsilon(string head)
{
	map< string,vector<vector<string> > >::iterator it1 = production.find(head);
	for(vector<vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
	{		
		for(vector<string>::iterator it3=it2->begin(); it3 != it2->end(); it3++)
		{
			if((*it3) == "epsilon")	
				return true;
		}
	}	
	return false;
}
bool hasTerminal(vector<vector<string> >::iterator rule)
{
	for(vector<string>::iterator it1 = rule->begin(); it1 != rule->end(); it1++)
	{
		bool terminal = true;
		for(vector<string>::iterator it2 = NT_namelist.begin(); it2 != NT_namelist.end(); it2++)
		{
			if(*it1 == *it2)
			{
				terminal = false;
				break;
			}	
		}
		if(terminal)
			return true;
	}
	return false;
}

void getFirst(non_terminal* NT_array[])
{
	bool done = false;
	while(!done)
	{
		done = true;
		for(map< string,vector<vector<string> > >::iterator it1 = production.begin(); it1 != production.end(); it1++)
		{
			for(vector<vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
			{		
				for(vector<string>::iterator it3=it2->begin(); it3 != it2->end(); it3++)
				{				
					if(addFirst(it1->first,*it3, NT_array))
					{
						done = false;
					}
					if(isNullable(*it3,NT_array))
						continue;
					else
						break;					
				}
			}
		}		
	}	
}
bool addFirst(string head, string body_symbol, non_terminal* NT_array[])
{
	int num1, num2;
	bool terminal = true;
	bool change;
	for(vector<string>::iterator it = NT_namelist.begin(); it != NT_namelist.end(); it++)
	{
		if(*it == body_symbol)
		{
			terminal = false;
			break;			
		}	
	}
	for(int i=0; i<NT_num; i++)
	{
		if(NT_array[i]->name == head)
			num1 = i;
	}
	if(!terminal)
	{
		for(int i=0; i<NT_num; i++)
		{						
			if(NT_array[i]->name == body_symbol)
				num2 = i;
		}		
		change = unionFirst(num1,num2,NT_array);
	}
	else
	{
		bool new_symbol= true;
		if( body_symbol == "epsilon")
			change = false;
		else
		{
			for(set<string>::iterator it = NT_array[num1]->First.begin(); it != NT_array[num1]->First.end(); it++)
			{
				if(*it == body_symbol)
				{
					change = false;
					new_symbol = false;
					break;
				}				
			}
			if(new_symbol)
			{
				NT_array[num1]->First.insert(body_symbol);
				change = true;
			}
		}
	}
	return change;
}
bool isNullable(string variable, non_terminal* NT_array[])
{
	for(int i=0; i<NT_num; i++)
	{
		if(NT_array[i]->name == variable)
			return NT_array[i]->nullable;
	}
	return false;
}
bool unionFirst(int num1, int num2, non_terminal* NT_array[])
{	
	bool change = false;
	for (set<string>::iterator it1 = NT_array[num2]->First.begin(); it1 != NT_array[num2]->First.end(); it1++)
	{
		bool match = false;
		for(set<string>::iterator it2 = NT_array[num1]->First.begin(); it2 != NT_array[num1]->First.end(); it2++)
		{
			if(*it1 == *it2)
			{
				match = true;
				break;
			}	
		}
		if(!match)
		{
			NT_array[num1]->First.insert(*it1);
			change = true;
		}	
	}
	return change;
}

void getFollow(non_terminal* NT_array[])
{
	bool done = false;
	int k = 0;
	
	while(!done)
	{
		/*
		if(k==1)
		{
			for(int i=0; i<NT_num; i++)
			{
				cout<<NT_array[i]->name<<"  ";
				for(set<string>::iterator it = NT_array[i]->Follow.begin(); it != NT_array[i]->Follow.end(); it++)
					cout<<*it<<" ";
				cout<<endl;	
			}
		}
		*/
		
		done = true;
		for(map< string,vector<vector<string> > >::iterator it1 = production.begin(); it1 != production.end(); it1++)
		{
			for(vector<vector<string> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
			{		
				for(vector<string>::iterator it3=it2->begin(); it3 != it2->end(); it3++)
				{				
					if(it3+1 == it2->end())
					{
						if( addFollow1(it1->first,*it3,NT_array) )
							done = false;
					}	
					else
					{
						if( addFollow2(*it3,*(it3+1),NT_array) )
							done = false;					
					}
				}
			}
		}

	}	
}
bool addFollow1(string head, string body_symbol, non_terminal* NT_array[])
{
	int num1, num2;
	bool terminal = true;
	bool change;
	for(vector<string>::iterator it = NT_namelist.begin(); it != NT_namelist.end(); it++)
	{
		if(*it == body_symbol)
		{
			terminal = false;
			break;			
		}	
	}
	if(!terminal)
	{
		for(int i=0; i<NT_num; i++)
		{						
			if(NT_array[i]->name == head)
				num1 = i;
			if(NT_array[i]->name == body_symbol)
				num2 = i;
		}		
		change = unionFollow1(num1,num2,NT_array);
		//if(change)
		//	cout<<"Add follow "<<NT_array[num1]->name<<" to follow "<<NT_array[num2]->name<<endl;
	}
	else
		change = false;
	
	return change;
}
bool addFollow2(string front, string back, non_terminal* NT_array[])
{
	int num1, num2;
	bool terminal_front = true;
	bool terminal_back = true;
	bool change;
	for(vector<string>::iterator it = NT_namelist.begin(); it != NT_namelist.end(); it++)
	{
		if(*it == front)
			terminal_front = false;
		if(*it == back)
			terminal_back = false;
	}
	
	if(terminal_front)
		change = false;
	else
	{
		for(int i=0; i<NT_num; i++)
		{
			if(NT_array[i]->name == front)
				num1 = i;
		}
		if(!terminal_back)
		{
			for(int i=0; i<NT_num; i++)
			{						
				if(NT_array[i]->name == back)
					num2 = i;
			}		
			change = unionFollow2(num1,num2,NT_array);
			//if(change)
			//	cout<<"Add first "<<NT_array[num2]->name<<" to follow "<<NT_array[num1]->name<<endl;
		}
		else
		{
			bool new_symbol= true;
			if( back == "epsilon")
				change = false;
			else
			{
				for(set<string>::iterator it = NT_array[num1]->Follow.begin(); it != NT_array[num1]->Follow.end(); it++)
				{
					if(*it == back)
					{
						change = false;
						new_symbol = false;
						break;
					}
				}
				if(new_symbol)
				{
					NT_array[num1]->Follow.insert(back);
					change = true;
					//if(change)
					//	cout<<"Add "<<back<<" to follow "<<NT_array[num1]->name<<endl;
				}
			}
		}
	}
	
	return change;
}
bool unionFollow1(int num1, int num2, non_terminal* NT_array[])
{	
	bool change = false;
	for (set<string>::iterator it1 = NT_array[num1]->Follow.begin(); it1 != NT_array[num1]->Follow.end(); it1++)
	{
		bool match = false;
		for(set<string>::iterator it2 = NT_array[num2]->Follow.begin(); it2 != NT_array[num2]->Follow.end(); it2++)
		{
			if(*it1 == *it2)
			{
				match = true;
				break;
			}	
		}
		if(!match)
		{
			NT_array[num2]->Follow.insert(*it1);
			change = true;
		}	
	}
	return change;
}
bool unionFollow2(int num1, int num2, non_terminal* NT_array[])
{	
	bool change = false;
	for (set<string>::iterator it1 = NT_array[num2]->First.begin(); it1 != NT_array[num2]->First.end(); it1++)
	{
		bool match = false;
		for(set<string>::iterator it2 = NT_array[num1]->Follow.begin(); it2 != NT_array[num1]->Follow.end(); it2++)
		{
			if(*it1 == *it2)
			{
				match = true;
				break;
			}	
		}
		if(!match)
		{
			NT_array[num1]->Follow.insert(*it1);
			change = true;
		}	
	}
	return change;
}
bool isTerminal(string symbol)
{
	bool terminal = true;
	for(vector<string>::iterator it = NT_namelist.begin(); it != NT_namelist.end(); it++)
	{
		if(*it == symbol)
		{
			terminal = false;
			break;			
		}	
	}
	return terminal;
}
int findNTnum(string symbol, non_terminal* NT_array[])
{
	int NTnum;
	for(int i=0; i<NT_num; i++)
	{
		if(NT_array[i]->name == symbol)
			NTnum = i;
	}
	return NTnum;
}

string findToken(string label,string lexeme)
{
	string token;
	
	if(label == "<Identifier>")
		token = "id";
	else if(label == "<Number>")
		token = "num";
	//else if(label == "<Char>")
	else 
		token = lexeme;
	
	return	token;
}

#if DEBUG
void buildParseTree(node* current)
{
	if(isTerminal(current->name));
	{
		if( current->name == reader->name )
		{
			addNode2(current,reader->name);
			if(reader->value != "NONE")
				addNode2(current->childnode[0],reader->value);	//set the iterator of tokenlist
			reader++;
		}
		else
		{
			cout<<"Error. Can't build parse tree.\n";
			exit(0);
		}	
	}
	else
	{
		vector<string>::iterator it1 = findProdStart(current->name,reader->name);
		vector<string>::iterator it2 = findProdEnd(current->name,reader->name);
		addNode1(current,it1,it2);
			
		for(vector<node*>::iterator it3 = current->childnode.begin(); it3 != current->childnode.end();it3++)	//walk through children nodes
		{
			buildParseTree(it3);	
		}
	}	
}
void addNode1(node* current, vector<string>::iterator it1, vector<string>::iterator it2)
{
	for(vector<string>::iterator it = it1; it != it2; it++)
	{
		node* x = new node(*it);
		current->childnode.push_back(x);	//childnode[0]裡面放的就是新節點的指標
	}
	
}
void addNode2(node* current, string value)
{
	node* x = new node(value);
	current->childnode.push_back(x);	//childnode[0]裡面放的就是新節點的指標	
}
vector<string>::iterator findProdStart(string non_terminal, string terminal)
{
	map<string, map<string, vector<string> > >::iterator it1 = LLtable.find(non_terminal)
	map<string, vector<string> >::iterator it2 = it1->second.find(terminal)
	vector<string>::iterator it3 = it2->second.begin();
	
	return it3;
}

vector<string>::iterator findProdEnd(string non_terminal, string terminal)
{
	map<string, map<string, vector<string> > >::iterator it1 = LLtable.find(non_terminal)
	map<string, vector<string> >::iterator it2 = it1->second.find(terminal)
	vector<string>::iterator it3 = it2->second.end();
	
	return it3;
}
#endif
void setSymbolTable(void)
{
	ofstream symbolFile("symbol.txt");
	ifstream tokenFile("token.txt");	
	char buff[100];
	int scope = 0;
	string type;
	vector<string> symbolList;
	
	tokenFile.getline(buff,100);		//filter the 1st line
	while(tokenFile.getline(buff,100))
	{
		istringstream stream(buff);
		string temp, lexeme, label;
		int count = 0;
		if(buff[0] == 'L')
			continue;
		else
		{		
			while(stream >> temp)
			{
				if(count == 0)
					label = temp;
				else if(count == 2)
					lexeme = temp;
				count++;	
			}
			if(lexeme == "int" || lexeme == "char")
				type = lexeme;
			else if(label == "<Identifier>")
			{
				bool symbolExist = false;
				for(vector<string>::iterator it = symbolList.begin(); it != symbolList.end(); it++)
				{
					if(*it == lexeme)
					{
						symbolExist = true;
						break;
					}
				}
				if(!symbolExist)
				{
					symbolList.push_back(lexeme);
					symbolFile<<lexeme<<"\tid"<<"\t"<<type<<"\t"<<scope<<endl;
				}	
			}
			else if(lexeme == "(" || lexeme == "{" )
				scope++;
			else if(lexeme == ")" || lexeme == "}" )
				scope--;
		}
	}
	tokenFile.close();
}

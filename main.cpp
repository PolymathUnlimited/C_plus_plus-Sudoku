#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <csignal>

using namespace std;

// GLOBAL VARIABLES

// board stack (for undo/redo)
int boardStack[9][9][89];
int stackIndex = 0;
int maxIndex = 0;

// file directory 
int numSaves = 0;
string* savedGames = NULL;

// initial board configuration (in case there is no autosave file)
int board[9][9] =
{
	{-1, 0, 0,-8, 0, 0,-6,-5, 0},
	{ 0, 0, 0,-9,-1, 0, 0,-2, 0},
	{ 0,-8, 0, 0,-5, 0,-7, 0,-9},
	{ 0, 0, 0, 0, 0, 0, 0,-9, 0},
	{ 0,-5,-3, 0,-4, 0,-1,-7, 0},
	{ 0,-4, 0, 0, 0, 0, 0, 0, 0},
	{-5, 0,-2, 0,-9, 0, 0,-3, 0},
	{ 0,-9, 0, 0,-7,-5, 0, 0, 0},
	{ 0,-7,-6, 0, 0,-2, 0, 0,-5},
};

// function to push a board state onto the stack
void push(int board[9][9])
{
	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			boardStack[x][y][stackIndex] = board[x][y];
	++stackIndex;
	maxIndex = stackIndex;
}

// function to clear the console
void clear()
{
#ifdef _WIN32
	system("cls"); // windows command to clear the console
#else
	system("clear"); // unix command to clear the console
#endif
}

// function to save a game
bool save(string filename, int board[9][9])
{
	// output file stream object
	ofstream fout;

	// open a file
	fout.open(filename);

	// check for errors
	if (fout.fail())
	{
		fout.close();
		return false;
	}

	// write the board state to the file
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			fout << board[x][y] << " ";
		}
		fout << endl;
	}

	// close file and return
	fout.close();
	return true;
}

// function to load a game from a file
bool load(string filename, int board[9][9])
{
	// input file stream object
	ifstream fin;

	// open a file
	fin.open(filename);

	// check for errors
	if (fin.fail())
	{
		fin.close();
		return false;
	}

	// read data from the file
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			fin >> board[x][y];
		}
	}

	// close file and return
	fin.close();
	return true;
}

// function to draw a sudoku board in the console
void draw(int board[9][9])
{
	// set text color to gray
	cout << "\033[38;2;150;150;150m";

	// print column markers
	cout << "   1 2 3   4 5 6   7 8 9" << endl << endl;

	// print rows
	for (int y = 0; y < 9; ++y)
	{
		// set text color to gray
		cout << "\033[38;2;150;150;150m";

		// print horizontal subgrid lines
		if (y == 3 || y == 6) cout << "   ------+-------+------" << endl;

		// print row markers
		cout << char('A' + y) << "  ";

		// print each number in the row
		for (int x = 0; x < 9; ++x)
		{
			// print horizontal subgrid lines
			if (x == 3 || x == 6) cout << "\033[38;2;150;150;150m| ";

			// if the number is a starting number, change the text color to green
			if (board[x][y] < 0) cout << "\033[38;2;0;255;0m";

			// otherwise, restore default settings
			else cout << "\033[0m";

			// print the number
			if (board[x][y] != 0) cout << abs(board[x][y]) << " ";
			else cout << "  ";
		}

		// new line for the new row
		cout << endl;
	}

	// restore default color settings
	cout << "\033[0m";
}

// function to check if a move is legal
bool isLegal(int board[9][9], int x, int y, int value) // arguments go in the parentheses
{
	// check to make sure we are on the board
	if (x < 0 || x >= 9 || y < 0 || y >= 9) return false;

	// check to make sure the value is valid
	if (value < 0 || value > 9) return false;

	// make sure setting a square to zero is always legal
	if (value == 0) return true;

	// make it illegal to modify starting squares
	if (board[x][y] < 0) return false;

	// check row and column
	for (int x = 0; x < 9; ++x)
		if (abs(board[x][y]) == value) return false;
	
	for (int y = 0; y < 9; ++y)
		if (abs(board[x][y]) == value) return false;

	// check subgrid
	int subgrid_x = (x / 3) * 3;
	int subgrid_y = (y / 3) * 3;

	for (int x = subgrid_x; x < subgrid_x + 3; ++x)
		for (int y = subgrid_y; y < subgrid_y + 3; ++y)
			if (abs(board[x][y]) == value) return false;

	// if the move is not illegal, then it is legal
	return true;
}

// function to solve a sudoku board
bool solve(int board[9][9])
{
	// find an empty square
	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			if (board[x][y] == 0)
			{
				// loop through all legal values for the square
				for (int i = 1; i <= 9; ++i)
					if (isLegal(board, x, y, i))
					{
						board[x][y] = i;

						// check to see if we have found the right value by recursively calling solve
						if (solve(board)) return true;
					}

				// if we can't find a value for the square, then backtrack
				board[x][y] = 0;
				return false;
			}

	// if there are no empty sqauares, then the board is solved
	return true;
}

// function to fill a board with random valid values
bool randFill(int board[9][9])
{
	// find an empty square
	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			if (board[x][y] == 0)
			{
				int values[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
				int numValues = 9;

				// try values in a random order
				while (numValues)
				{
					// randomly choose a value
					int index = rand() % numValues;
					int value = values[index];

					// remove the chosen value from the array
					--numValues;
					for (int i = index; i < numValues; ++i)
						values[i] = values[i + 1];

					// if the value is legal, then try it and recurse
					if (isLegal(board, x, y, value))
					{
						board[x][y] = value;
						if (randFill(board)) return true;
					}
				}

				// if no values were found, then backtrack
				board[x][y] = 0;
				return false;
			}

	// if no squares are empty, then we are done
	return true;
}

// function to detect if a board has multiple solutions
bool multiSolve(int board[9][9], int& numSolutions)
{
	// find an empty square
	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			if (board[x][y] == 0)
			{
				// loop through all legal values for the square
				for (int i = 1; i <= 9; ++i)
					if (isLegal(board, x, y, i))
					{
						board[x][y] = i;

						// recursive call
						if (multiSolve(board, numSolutions)) return true;
					}

				// backtrack
				board[x][y] = 0;
				return false;
			}

	// if the board is full, increment solution counter and return
	++numSolutions;
	return numSolutions >= 2;
}

// function to reduce a full board while ensuring solution uniqueness
bool generate(int board[9][9], int numEntries)
{
	// if there are multiple solutions, then backtrack
	int tempBoard[9][9];
	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			tempBoard[x][y] = board[x][y];

	int numSolutions = 0;
	if (multiSolve(tempBoard, numSolutions)) return 0;

	// make a list of all non-empty squares
	int xList[81];
	int yList[81];
	int numNonEmpties = 0;

	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			if (board[x][y] != 0)
			{
				xList[numNonEmpties] = x;
				yList[numNonEmpties] = y;
				++numNonEmpties;
			}

	// if we have removed enough entries, the we were done
	if (numNonEmpties <= numEntries) return true;

	// iterate through all non-empty squares
	while (numNonEmpties)
	{
		// pick a random non-empty square
		int index = rand() % numNonEmpties;
		int x = xList[index];
		int y = yList[index];

		// remove that square from the list
		--numNonEmpties;
		for (int i = index; i < numNonEmpties; ++i)
		{
			xList[i] = xList[i + 1];
			yList[i] = yList[i + 1];
		}

		// save the value of the square and remove it from the board
		int value = board[x][y];
		board[x][y] = 0;

		// recursively call the generate function
		if (generate(board, numEntries)) return true;

		// if the call failed, then reset the square to its original value before continuing
		board[x][y] = value;
	}

	// if no non-empty squares can be safely removed, then we need to backtrack
	return false;
}

// function to reset a board to its initial state
void reset(int board[9][9])
{
	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y)
			if (board[x][y] > 0) board[x][y] = 0;
}

// function to alphebetize an array of strings
void alphebetize(string strings[], int numStrings)
{
	// selection sort
	for (int i = 0; i < numStrings; ++i)
	{
		// determine which element we need to swap with
		int swapIndex = i;
		for (int j = i; j < numStrings; ++j)
		{
			if (strings[j] < strings[swapIndex])
				swapIndex = j;
		}

		// perform the swap
		string temp = strings[swapIndex];
		strings[swapIndex] = strings[i];
		strings[i] = temp;
	}
}

// signal handler
void signalHandler(int signal)
{
	// code that runs before exit will go here.

	save("autosave.txt", board); // save the board to an autosave file

	// save contents of the directory
	ofstream fout("directory.txt");
	fout << numSaves << endl;
	for (int i = 0; i < numSaves; ++i)
		fout << savedGames[i] << endl;
	fout.close();

	// deallocate memory from saved games
	if (savedGames)
	{
		delete[] savedGames;
		savedGames = NULL;
	}
}

// menu function
string menu()
{
	// clear the console
	clear();

	// display the menu
	cout << "Welcome to Polymath Sudoku!  Please choose from the options below:" << endl << endl;
	cout << "1: Continue Game" << endl;
	cout << "2: New Game" << endl;
	cout << "3: Save Game" << endl;
	cout << "4: Load Game" << endl;
	cout << "5: Delete Save" << endl;
	cout << "6: Help" << endl;
	cout << "7: Exit" << endl << endl;
	cout << ">> ";

	// get user input
	string selection;
	getline(cin, selection);

	// process input
	if (selection == "1" || selection == "Continue Game" || selection == "continue game") return "";
	if (selection == "2" || selection == "New Game" || selection == "new game")
	{
		while (true)
		{
			// clear the console
			clear();

			// give the player some options
			cout << "Select difficulty:" << endl;
			cout << "(enter 0 or back to go back to main menu)" << endl << endl;
			cout << "1: Easy" << endl;
			cout << "2: Medium" << endl;
			cout << "3: Hard" << endl << endl;
			cout << ">> ";

			// get input
			getline(cin, selection);

			// process input
			if (selection == "0" || selection == "Back" || selection == "back") return menu();
			if (selection == "1" || selection == "Easy" || selection == "easy") return "new easy";
			if (selection == "2" || selection == "Medium" || selection == "medium") return "new medium";
			if (selection == "3" || selection == "Hard" || selection == "hard") return "new hard";
		}
	}
	if (selection == "3" || selection == "Save Game" || selection == "save game")
	{
		while (true)
		{
			// clear the console
			clear();

			// list saved games
			cout << "Select a file to save to (or enter a new filename):" << endl;
			cout << "(enter 0 or back to go back to main menu)" << endl << endl;
			for (int i = 0; i < numSaves; ++i)
				cout << i + 1 << ": " << savedGames[i] << endl;
			cout << endl << ">> ";

			// get user input
			getline(cin, selection);
			if (selection == "0" || selection == "Back" || selection == "back") return menu();

			// see if the user gave us a number
			bool isNumber = true;
			for (int i = 0; i < selection.length(); ++i)
				if (selection[i] < '0' || selection[i] > '9')
				{
					isNumber = false;
					break;
				}
			if (isNumber)
			{
				int index = stoi(selection) - 1;
				if (index < numSaves) selection = savedGames[index];
			}

			// return command
			return "save " + selection;
		}
	}
	if (selection == "4" || selection == "Load Game" || selection == "load game")
	{
		while (true)
		{
			// clear the console
			clear();

			// list saved games
			cout << "Select a file to load:" << endl;
			cout << "(enter 0 or back to go back to main menu)" << endl << endl;
			for (int i = 0; i < numSaves; ++i)
				cout << i + 1 << ": " << savedGames[i] << endl;
			cout << endl << ">> ";

			// get user input
			getline(cin, selection);
			if (selection == "0" || selection == "Back" || selection == "back") return menu();

			// see if the user gave us a number
			bool isNumber = true;
			for (int i = 0; i < selection.length(); ++i)
				if (selection[i] < '0' || selection[i] > '9')
				{
					isNumber = false;
					break;
				}
			if (isNumber)
			{
				int index = stoi(selection) - 1;
				if (index < numSaves) selection = savedGames[index];
			}

			// check to make sure filename is in the directory before returning
			bool inDirectory = false;
			for (int i = 0; i < numSaves; ++i)
				if (savedGames[i] == selection) return "load " + selection;
		}
	}
	if (selection == "5" || selection == "Delete Save" || selection == "delete save")
	{
		while (true)
		{
			// clear the console
			clear();

			// list saved games
			cout << "Select a file to delete:" << endl;
			cout << "(enter 0 or back to go back to main menu)" << endl << endl;
			for (int i = 0; i < numSaves; ++i)
				cout << i + 1 << ": " << savedGames[i] << endl;
			cout << endl << ">> ";

			// get user input
			getline(cin, selection);
			if (selection == "0" || selection == "Back" || selection == "back") return menu();

			// see if the user gave us a number
			bool isNumber = true;
			for (int i = 0; i < selection.length(); ++i)
				if (selection[i] < '0' || selection[i] > '9')
				{
					isNumber = false;
					break;
				}
			if (isNumber)
			{
				int index = stoi(selection) - 1;
				if (index < numSaves) selection = savedGames[index];
			}

			// check to make sure filename is in the directory before returning
			bool inDirectory = false;
			for (int i = 0; i < numSaves; ++i)
				if (savedGames[i] == selection) return "delete " + selection;
		}
	}
	if (selection == "6" || selection == "Help" || selection == "help")
	{
		while (true)
		{
			// clear the console
			clear();

			// output help message
			cout <<
				"Welcome to Polymath Sudoku!  Sudoku is a game played by placing the numbers 1 through 9 on a\n"
				"9x9 grid.  The goal of the game is to fill all squares such that each number appears only once\n"
				"in each row, once in each column, and once in each subgrid." << endl << endl;
			cout <<
				"This application features a console interface allowing you to enter commands for the game to\n"
				"run.  The game will display a sudoku board, after which you may type a command.  The commands\n"
				"you can run are as follows:" << endl << endl;
			cout <<
				"You can set the value of a square using the \"set\" command. For example, typing the command\n"
				"\"set b7 4\" into the console will place the value 4 into the square b7." << endl << endl;
			cout <<
				"You can load, save, and delete games directly in the game's console interface with the \"load\"\n"
				"\"save\" and \"delete\" commands, respectively.  For example, the command \"save saveGame1\" will\n"
				"save the current board to a file called \"saveGame1.\"  If such a file already exists in the\n"
				"file directory, it will be saved to that file.  If no such file exists, a new file will be\n"
				"created.  You can view a list of all available save files with the \"list saves\" command." << endl << endl;
			cout <<
				"You can view the solution to the current puzzle with the \"solve\" command.  If you want to\n"
				"view the correct value of a particular square, you can use the \"hint\" command.  For example,\n"
				"the command \"hint f8\" will show the correct value to be placed on square f8." << endl << endl;
			cout <<
				"You can generate a new puzzle directly in the game's console using the \"new\" command.  For\n"
				"example, the command \"new medium\" will generate a new puzzle of medium difficulty.  Available\n"
				"difficulties are \"easy\" \"medium\" and \"hard.\"" << endl << endl;
			cout <<
				"You can undo moves with the \"undo\" command.  You can also redo moves with the \"redo\" command." << endl << endl;
			cout <<
				"You can access the main menu using the \"menu\" command." << endl << endl;
			cout <<
				"You can exit the game using the \"exit\" command.  When the game exits, your progress will be\n"
				"saved in an auto-save file that will be loaded the next time the game launches." << endl << endl;
			cout << "To go back to the main menu, enter \"0\" or \"back\":" << endl << endl;
			cout << ">> ";

			getline(cin, selection);
			if (selection == "0" || selection == "Back" || selection == "back") return menu();
		}
	}
	if (selection == "7" || selection == "Exit" || selection == "exit") return "exit";

	// if input is invalid, then simply call menu again
	return menu();
}

// main function
int main()
{
	// seed the random number generator
	srand(time(NULL));

	// register the signal handler
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
#ifdef _WIN32
	signal(SIGBREAK, signalHandler);
#endif

	// stringstream object for writing data to the console on future iterations of the game loop
	stringstream console;

	// load the file directory
	ifstream fin("directory.txt");
	if (!fin.fail())
	{
		fin >> numSaves;
		
		// allocate memory for savedGames
		savedGames = new string[numSaves];

		// read savedGames from the directory
		for (int i = 0; i < numSaves; ++i)
			fin >> savedGames[i];
	}
	fin.close();

	// alphebetize directory (for redundency)
	alphebetize(savedGames, numSaves);

	// the game loop will run while running is true
	bool running = true;

	// load the autosave file
	load("autosave.txt", board);
	push(board);

	// game loop
	bool menuCommand = true;
	while (running)
	{
		// clear the console
		clear();

		// get a command
		string command;
		if (menuCommand)
		{
			// go to main menu
			command = menu();
			menuCommand = false;

			// if menu command was save or delete, then stay in the menu
			if (command.substr(0, 4) == "save" || command.substr(0, 6) == "delete") menuCommand = true;
		}
		else
		{
			// generate output (display a sudoku board)
			cout << endl;
			draw(board);

			// print contents of the stringstream to the console, then clear the stringstream
			cout << endl << console.str() << endl;
			console.str("");

			// prompt the user for input
			cout << endl << "Enter a command: ";

			// get user input
			getline(cin, command);
		}

		// process user input
		if (command.substr(0, 3) == "set")
		{
			int y = command[4] - 'a';
			int x = command[5] - '1';
			int value = command[7] - '0';
			
			if (isLegal(board, x, y, value))
			{
				board[x][y] = value;
				push(board);
			}
			else
			{
				console << "The move is illegal!" << endl;
			}
		}
		if (command.substr(0, 4) == "load")
		{
			// if the user gave a filename, then load using the filename given
			if (command.length() > 4)
			{
				string filename = command.substr(4);
				// delete spaces
				while (filename.length() > 0 && filename[0] == ' ') filename.erase(0, 1);
				// load from the file
				load(filename, board);
				
				// reset the board stack
				stackIndex = 0;
				push(board);
			}
			// otherwise, use default file
			else load("default.txt", board);
		}
		if (command.substr(0, 4) == "save")
		{
			// if the user gave a filename, then load using the filename given
			if (command.length() > 4)
			{
				string filename = command.substr(4);
				// delete spaces
				while (filename.length() > 0 && filename[0] == ' ') filename.erase(0, 1);
				// load from the file
				save(filename, board);

				// check if the save is already in the directory
				bool inDirectory = false;
				for (int i = 0; i < numSaves; ++i)
					if (savedGames[i] == filename) inDirectory = true;

				// add the filename to directory
				if (!inDirectory)
				{
					// allocate additional memory for savedGames
					string* temp = new string[numSaves];
					for (int i = 0; i < numSaves; ++i) temp[i] = savedGames[i];
					if (savedGames) delete[] savedGames;
					savedGames = new string[numSaves + 1];
					for (int i = 0; i < numSaves; ++i) savedGames[i] = temp[i];
					if (temp)
					{
						delete[] temp;
						temp = NULL;
					}

					// add the filename to the directory
					savedGames[numSaves] = filename;
					++numSaves;
				}

				// re-alphebetize the directory
				alphebetize(savedGames, numSaves);
			}
			// otherwise, use default file
			else save("default.txt", board);
		}
		if (command == "list saves")
		{
			console << "Saved Games: " << endl;
			// list contents of the directory
			for (int i = 0; i < numSaves; ++i)
				console << "\t" << savedGames[i] << endl;
		}
		if (command.substr(0, 6) == "delete")
		{
			if (command.length() > 6)
			{
				// get the filename
				string filename = command.substr(6);
				// delete leading spaces
				while (filename.length() > 0 && filename[0] == ' ')
					filename.erase(0, 1);
				// remove filename from the directory
				for (int i = 0; i < numSaves; ++i)
				{
					if (savedGames[i] == filename)
					{
						for (int j = i + 1; j < numSaves; ++j)
							savedGames[j - 1] = savedGames[j];
						--numSaves;
					}
				}
				// delete file from the hard drive
				remove(filename.c_str());
			}
		}
		if (command == "solve")
		{
			reset(board);
			if(!solve(board))
				console << "No solutions found!";
			push(board);
		}
		if (command.substr(0, 4) == "hint")
		{
			int y = command[5] - 'a';
			int x = command[6] - '1';

			// copy board into a temporary array
			int temp[9][9];
			for (int x = 0; x < 9; ++x)
				for (int y = 0; y < 9; ++y)
					temp[x][y] = board[x][y];

			// solve the temp board
			reset(temp);
			solve(temp);

			// give the hint to the user
			console << "The value of square " << char(y + 'a') << char(x + '1') << " is " << temp[x][y] << endl;
		}
		if (command.substr(0, 3) == "new")
		{
			// clear the board
			for (int x = 0; x < 9; ++x)
				for (int y = 0; y < 9; ++y)
					board[x][y] = 0;

			// generate a new puzzle
			randFill(board);
			if (command.substr(4) == "easy") generate(board, 35 + rand() % 5);
			if (command.substr(4) == "medium") generate(board, 30 + rand() % 5);
			if (command.substr(4) == "hard") generate(board, 25 + rand() % 5);

			// flip signs to denote starting squares
			for (int x = 0; x < 9; ++x)
				for (int y = 0; y < 9; ++y)
					board[x][y] = -board[x][y];

			// reset the board stack
			stackIndex = 0;
			push(board);
		}
		if (command == "menu")
		{
			menuCommand = true;
		}
		if (command == "undo" && stackIndex > 1)
		{
			--stackIndex;
			for (int x = 0; x < 9; ++x)
				for (int y = 0; y < 9; ++y)
					board[x][y] = boardStack[x][y][stackIndex - 1];
		}
		if (command == "redo" && stackIndex < maxIndex)
		{
			for (int x = 0; x < 9; ++x)
				for (int y = 0; y < 9; ++y)
					board[x][y] = boardStack[x][y][stackIndex];
			++stackIndex;
		}
		if (command == "exit")
		{
			// this code executes if the command entered by the user is "exit"
			save("autosave.txt", board); // save the board to an autosave file
			running = false; // now, the loop will exit after it is done running this iteration

			// save contents of the directory
			ofstream fout("directory.txt");
			fout << numSaves << endl;
			for (int i = 0; i < numSaves; ++i)
				fout << savedGames[i] << endl;
			fout.close();
		}
	}

	// deallocate savedGames memory
	if (savedGames)
	{
		delete[] savedGames;
		savedGames = NULL;
	}

	// end program
	return 0;
}
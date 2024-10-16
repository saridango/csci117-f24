// COMPILE: g++ -std=c++11 prog3.cpp -o prog3
// RUN: ./prog3

#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <vector>
#include <sstream>
#include <stdexcept>

using namespace std;

struct Node {
    string id;   // Variable name
    string type; // Variable type (int or double)
    int val;     // Variable value
};

class SymbolTable {
public:
    void addVariable(const string& id, const string& type) {
        Node node;
        node.id = id;
        node.type = type;
        node.val = 0;
        table.push_back(node);
    }

    int getValue(const string& id) {
        for (const auto& node : table) {
            if (node.id == id) {
                return node.val;
            }
        }
        throw runtime_error("Semantic error: Undeclared variable '" + id + "'");
    }

    void setValue(const string& id, int value) {
        for (auto& node : table) {
            if (node.id == id) {
                node.val = value;
                return;
            }
        }
        throw runtime_error("Semantic error: Undeclared variable '" + id + "'");
    }

private:
    vector<Node> table;
};

class Interpreter {
public:
    Interpreter(const string& filename);
    void parse();

private:
    string progStr; // String for reading input expression (program)
    int indexx = 0; // Global index for program string
    SymbolTable symbolTable; // Symbol table for variables

    void removeSpaces();
    string readFileContent(const string& filename);
    void prog();
    void declarations();
    void declaration();
    string type();
    void id_list(string varType);
    string id();
    void statements();
    void statement();
    void assign_st();
    void print_st();
    int Exp();
    int Term();
    int Exp2(int);
    int Term2(int);
    int Fact();
    int Power();
};

Interpreter::Interpreter(const string& filename) {
    progStr = readFileContent(filename);
    removeSpaces();
}

void Interpreter::removeSpaces() {
    progStr.erase(remove_if(progStr.begin(), progStr.end(), ::isspace), progStr.end());
}

string Interpreter::readFileContent(const string& filename) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << infile.rdbuf(); // Read entire file into buffer
    infile.close();
    return buffer.str();
}

void Interpreter::prog() {
    if (progStr.substr(indexx, 7) == "program") {
        indexx += 7;
        declarations();
        if (progStr.substr(indexx, 5) == "begin") {
            indexx += 5;
            statements();
            if (progStr.substr(indexx, 3) == "end") {
                indexx += 3;
                cout << "'end' found, program executed successfully." << endl;
            } else {
                cerr << "Error: 'end' expected." << endl;
                exit(1);
            }
        } else {
            cerr << "Error: 'begin' expected." << endl;
            exit(1);
        }
    } else {
        cerr << "Error: 'program' expected at start." << endl;
        exit(1);
    }
}

void Interpreter::declarations() {
    while (indexx < progStr.length() && (progStr.substr(indexx, 3) == "int" || progStr.substr(indexx, 6) == "double")) {
        declaration();
    }
}

void Interpreter::declaration() {
    string varType = type(); // Get the type of the variable (int or double)
    id_list(varType); // Get the list of variables
    if (progStr[indexx] == ';') {
        indexx++; // Move past the ';'
    } else {
        cerr << "Error: ';' expected after declaration." << endl;
        exit(1);
    }
}

string Interpreter::type() {
    if (progStr.substr(indexx, 3) == "int") {
        indexx += 3;
        return "int";
    } else if (progStr.substr(indexx, 6) == "double") {
        indexx += 6;
        return "double";
    } else {
        cerr << "Error: type expected (int or double)." << endl;
        exit(1);
    }
}

void Interpreter::id_list(string varType) {
    while (true) {
        string varName = id();
        symbolTable.addVariable(varName, varType); // Add variable to symbol table
        if (progStr[indexx] == ',') {
            indexx++; // Move past the ','
        } else {
            break;
        }
    }
}

string Interpreter::id() {
    if (isalpha(progStr[indexx])) {
        string result;
        while (isalnum(progStr[indexx])) {
            result += progStr[indexx++];
        }
        return result;
    } else {
        cerr << "Error: identifier expected." << endl;
        exit(1);
    }
}

void Interpreter::statements() {
    while (progStr.substr(indexx, 3) != "end") {
        statement();
    }
}

void Interpreter::statement() {
    if (progStr.substr(indexx, 5) == "print") {
        print_st();
    } else {
        assign_st();
    }
}

void Interpreter::assign_st() {
    string varName = id(); // Get variable name
    if (progStr[indexx] == '=') {
        indexx++; // Move past the '='
        int value = Exp(); // Evaluate expression on right side
        symbolTable.setValue(varName, value); // Assign to variable in symbol table
    } else {
        cerr << "Error: '=' expected in assignment statement." << endl;
        exit(1);
    }
    if (progStr[indexx] == ';') {
        indexx++; // Move past the ';'
    } else {
        cerr << "Error: ';' expected after assignment." << endl;
        exit(1);
    }
}

void Interpreter::print_st() {
    indexx += 5; // Move past 'print'
    int value = Exp(); // Evaluate expression to be printed
    cout << value << endl;
    if (progStr[indexx] == ';') {
        indexx++; // Move past the ';'
    } else {
        cerr << "Error: ';' expected after print statement." << endl;
        exit(1);
    }
}

int Interpreter::Exp() {
    int result = Exp2(Term());
    return result;
}

int Interpreter::Term() {
    int result = Term2(Power());
    return result;
}

int Interpreter::Exp2(int inp) {
    int result = inp;
    while (indexx < progStr.length()) {
        char a = progStr[indexx++];
        if (a == '+') {
            result += Term(); // Continue processing with addition
        } else if (a == '-') {
            result -= Term(); // Continue processing with subtraction
        } else {
            indexx--; // Revert index if not an operator
            break;
        }
    }
    return result;
}

int Interpreter::Term2(int inp) {
    int result = inp;
    while (indexx < progStr.length()) {
        char a = progStr[indexx++];
        if (a == '*') {
            result *= Power(); // Continue processing with multiplication
        } else if (a == '/') {
            result /= Power(); // Continue processing with division
        } else {
            indexx--; // Go back a position if addition or subtraction is encountered
            break;
        }
    }
    return result;
}

int Interpreter::Power() {
    int result = Fact(); // Start with a factor
    while (indexx < progStr.length() && progStr[indexx] == '^') {
        indexx++; // Move past the '^'
        result = pow(result, Power()); // Handle right-associative power
    }
    return result;
}

int Interpreter::Fact() {
    if (progStr[indexx] == '(') {
        indexx++; // Skip '('
        int value = Exp(); // Evaluate expression inside parentheses
        indexx++; // Skip ')'
        return value;
    } else if (isdigit(progStr[indexx])) {
        int value = 0;
        while (isdigit(progStr[indexx])) {
            value = value * 10 + (progStr[indexx++] - '0');
        }
        return value;
    } else if (isalpha(progStr[indexx])) {
        string varName = id(); // Get variable name
        return symbolTable.getValue(varName); // Return variable value from symbol table
    } else {
        cerr << "Error: unexpected character in expression." << endl;
        exit(1);
    }
}

void Interpreter::parse() {
    prog();
}

int main() {
    Interpreter interpreter("lexical_error.txt"); // Change this to your input file
    try {
        interpreter.parse(); // Start parsing the program
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1; // Return an error code
    }
    return 0; // Successful execution
}

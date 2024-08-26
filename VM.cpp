#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <format>
#include <typeinfo>

using namespace std;
using umap = unordered_map<string,string>;

umap commandList() {
    umap out;
    out["push"] = "C_PUSH";
    out["pop"] = "C_POP";
    out["label"] = "C_LABEL";
    out["goto"] = "C_GOTO";
    out["if-goto"] = "C_IF";
    out["label"] = "C_LABEL";
    out["function"] = "C_FUNCTION";
    out["return"] = "C_RETURN";
    out["call"] = "C_CALL";

    out["add"] = "C_ARITHMETIC";
    out["neg"] = "C_ARITHMETIC";
    out["sub"] = "C_ARITHMETIC";
    out["eq"] = "C_ARITHMETIC";
    out["gt"] = "C_ARITHMETIC";
    out["lt"] = "C_ARITHMETIC";
    out["and"] = "C_ARITHMETIC";
    out["or"] = "C_ARITHMETIC";
    out["not"] = "C_ARITHMETIC";

    return out;
}

umap segmentList() {
    umap out;
    out["local"] = "LCL";
    out["argument"] = "ARG";
    out["this"] = "THIS";
    out["that"] = "THAT";
    out["temp"] = "TEMP";

    return out;
}

string commandType(string line) {
    string command;
    umap commands = commandList();
    for (char c : line) {
        if (c != ' ' && c != '\0') {
            command += c;
        } else if (command.size() == 0){
            continue;
        } else {
            break;
        }
    }
    return commands[command];
}

string arg1(string line) {
    string arg;
    if (commandType(line) == "C_ARITHMETIC") {
        for (char c : line) {
            if (c != ' ') {
                arg += c;
            }
        }
    } else {
        bool isArg = false;
        char prev = 0;
        umap seg = segmentList();
        for (char c : line) {
            if (isArg == false && (prev == ' ' && c != ' ')) {
                isArg = true;
            }
            if (isArg && c != ' ') {
                arg += c;
            } else if (isArg) {
                break;
            }
            prev = c;
        }
        if (arg == "local" || arg == "argument" || arg == "this" || arg == "that" || arg == "temp") {
            arg = seg[arg];
        }
        
    }
    return arg;
}

int arg2(string line) {
    string arg;
    for (char c : line) {
        if (c <= '9' && c >= '0') {
            arg += c;
        }
    }
    return stoi(arg);
}

string segmentInit(string fileName, string segment, int index) {
    string out;
    if (segment == "static") {
        string var = fileName + "." + to_string(index);
        out = "@" + var + "\n";
    } else if (segment == "constant") {
        out = "@" + to_string(index) + "\n";
    } else if (segment == "pointer") {
        if (index == 0) {
            out = "@THIS\n";
        } else {
            out = "@THAT\n";
        }
    } else if (segment == "TEMP") {
        out = "@" + to_string(5+index) + "\n";
    } else {
        out = "@" + segment + "\n";
        out += "A=M\n";
        for (int i = 0; i < index; i++) {
            out += "A=A+1\n";
        }
        
    }
    return out;
}

string push(string fileName, string segment, int index) {
    string out = "// push " + segment + " " + to_string(index) + "\n";
    out += segmentInit(fileName,segment,index);
    if (segment == "constant") {
        out += "D=A\n";
    } else {
        out += "D=M\n";
    }
    out += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    return out;
}

string top() {
    // Targets top of stack    
    return "@SP\nM=M-1\nA=M\n"; 
}

string pop(string fileName, string segment, int index) {
    string out = "// pop " + segment + " " + to_string(index) + "\n";
    out += top();
    out += "D=M\n";
    out += segmentInit(fileName,segment,index);
    out += "M=D\n";
    return out;
}

string writePushPop(string fileName, string command, string segment, int index) {
    if (command == "C_PUSH") {
        return push(fileName,segment,index);
    } 

    return pop(fileName,segment,index);

}

string addC() {
    string out = "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "D=M\n"
                 "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "M=D+M\n";

    return out;
}

string subC() {
    string out = "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "D=M\n"
                 "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "M=M-D\n";

    return out;
}

string negC() {
    string out = "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "M=-M\n"
                 "@SP\n"
                 "M=M+1\n";
    return out;
}

string eqC(int count) {
    string EQ = "EQ" + to_string(count);
    string NEQ = "NEQ" + to_string(count);
    string END = "ENDEQ" + to_string(count);
    string out = subC() +
                 "D=M\n"
                 "@" + EQ + "\n"
                 "D;JEQ\n"
                 "@" + NEQ + "\n"
                 "D;JNE\n"
                 "("+ NEQ + ")\n"
                 "@0\n"
                 "D=A\n"
                 "@SP\n"
                 "A=M\n"
                 "M=D\n"
                 "@SP\n"
                 "M=M+1\n"
                 "@" + END + "\n"
                 "0;JMP\n"
                 "("+ EQ + ")\n"
                 "@1\n"
                 "D=A\n"
                 "D=-D\n"
                 "@SP\n"
                 "A=M\n"
                 "M=D\n"
                 "@SP\n"
                 "M=M+1\n"
                 "("+ END + ")\n";
    return out;

}

string gtC(int count) {
    string GT = "GT" + to_string(count);
    string NGT = "NGT" + to_string(count);
    string END = "ENDGT" + to_string(count);
    string out = subC() +
                 "D=M\n"
                 "@" + GT + "\n"
                 "D;JGT\n"
                 "@" + NGT + "\n"
                 "D;JLE\n"
                 "("+ NGT + ")\n"
                 "@0\n"
                 "D=A\n"
                 "@SP\n"
                 "A=M\n"
                 "M=D\n"
                 "@SP\n"
                 "M=M+1\n"
                 "@" + END + "\n"
                 "0;JMP\n"
                 "("+ GT + ")\n"
                 "@1\n"
                 "D=A\n"
                 "D=-D\n"
                 "@SP\n"
                 "A=M\n"
                 "M=D\n"
                 "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n"
                 "("+ END + ")\n";
    return out;
}

string ltC(int count) {
    string LT = "LT" + to_string(count);
    string NLT = "NLT" + to_string(count);
    string END = "ENDLT" + to_string(count);
    string out = subC() +
                 "D=M\n"
                 "@" + LT + "\n"
                 "D;JLT\n"
                 "@" + NLT + "\n"
                 "D;JGE\n"
                 "("+ NLT + ")\n"
                 "@0\n"
                 "D=A\n"
                 "@SP\n"
                 "A=M\n"
                 "M=D\n"
                 "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n"
                 "@" + END + "\n"
                 "0;JMP\n"
                 "("+ LT + ")\n"
                 "@1\n"
                 "D=A\n"
                 "D=-D\n"
                 "@SP\n"
                 "A=M\n"
                 "M=D\n"
                 "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n"
                 "("+ END + ")\n";
    return out;
}

string andC(int count) {
    string AND = "AND" + to_string(count);
    string NAND = "NAND" + to_string(count);
    string END = "ENDAND" + to_string(count);
    string out = "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "D=M\n"
                 "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "M=D&M\n"
                 "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n";
    return out;
}

string orC(int count) {
    string OR = "OR" + to_string(count);
    string NOR = "NOR" + to_string(count);
    string END = "ENDOR" + to_string(count);
    string out = "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "D=M\n"
                 "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "M=D|M\n"
                 "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n";
    return out;
}

string notC(int count) {
    string T = "TRUE" + to_string(count);
    string F = "FALSE" + to_string(count);
    string END = "ENDNOT" + to_string(count);
    string out = "@SP\n"
                 "M=M-1\n"
                 "A=M\n"
                 "M=!M\n"
                 "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n";
    return out;

}

string writeArithmetic(string command, unordered_map<string,int> *count) {
    string out = "//" + command + "\n";
    if (command == "add") {
        out += addC();
        out += "@SP\nM=M+1\n";
                
    } else if (command == "sub") {
        out += subC();
        out += "@0\nD=A\n@SP\nM=M+1\nA=M\nM=D\n";
    } else if (command == "neg") {
        out += negC();
    } else if (command == "eq") {
        out += eqC((*count)[command]);
        (*count)[command]++;
    } else if (command == "gt") {
        out += gtC((*count)[command]);
        (*count)[command]++;
    } else if (command == "lt") {
        out += ltC((*count)[command]);
        (*count)[command]++;
    } else if (command == "and") {
        out += andC((*count)[command]);
        (*count)[command]++;
    } else if (command == "or") {
        out += orC((*count)[command]);
        (*count)[command]++;
    } else if (command == "not") {
        out += notC((*count)[command]);
        (*count)[command]++;
    }
    return out;
}

unordered_map<string,int> countInit() {
    unordered_map<string,int> out;
    out["eq"] = 0;
    out["gt"] = 0;
    out["lt"] = 0;
    out["and"] = 0;
    out["or"] = 0;
    out["not"] = 0;
    return out;
}

string outputName (string file) {
    string name;
    for (char c : file) {
        if (c == '.') {
            break;
        }
        name += c;
    }
    return name;
}

string writeLabel(string label) {
    
    return "(" + label + ")\n";
}

string removeWhite(string line) {
    if (line[0] != ' ' && line[0] != 9) {
        return line;
    }
    string out;
    bool hasStart = false;
    for (char c : line) {
        if (hasStart) {
            out += c;
        } else if (c != ' ' && c != 9) {
            hasStart = true;
            out += c;
        }
    }
    return out;
}

string writeGoto(string label) {
    string out = "//Goto " + label + "\n"
                 "@" + label + "\n" 
                 "0;JMP\n";
    return out;
}

string writeIfGoto(string label) {
    string out = "// If-goto " + label + "\n"
                "@SP\n" 
                 "M=M-1\n"
                 "A=M\n"
                 "D=M\n"
                 "@" + label + "\n"
                 "D;JNE\n";
    return out;
}

string writeFunction(string function, int nVars) {
    string out = "// Function " + function + "\n";
     out += writeLabel(function) + "\n";
    for (int i = 0; i < nVars; i++) {
        out += "@SP\n"
               "A=M\n"
               "M=0\n"
               "@SP\n"
               "M=M+1\n\n";
    }
    return out;
}

string writeCall(string function, int nArgs, int nCalls) {
    string out;
    string push = "@SP\n"
                  "A=M\n"
                  "M=D\n"
                  "@SP\n"
                  "M=M+1\n\n";
    string retAdd = function + "$ret." + to_string(nCalls);
    out = "// Call " + function + "\n";
    // push return address
    out += "@" + retAdd + "\nD=A\n" + push;
    // push LCL
    out += "@LCL\nD=M" + push;
    // push ARG
    out += "@ARG\nD=M" + push;
    // push THIS
    out += "@THIS\nD=M" + push;
    // push THAT
    out += "@THAT\nD=M" + push;
    // ARG and LCL repos
    out += "@SP\n"
           "D=M\n"
           "@LCL\n"
           "M=D\n"
           "@5\n"
           "D=D-A\n"
           "@" + to_string(nArgs) + "\n"
           "D=D-A\n"
           "@ARG\n"
           "M=D\n";
    out += writeGoto(function);
    out += writeLabel(retAdd);
    cout << "test 2" << endl;
    return out;
}

string funcName(string fileName, string function) {
    return outputName(fileName) + "." + function;
}

string writeReturn() {
    string out;
    out += "// Return\n"
           "@LCL\n"
           "D=M\n"
           "@R5\n"
           "M=D\n"
       // Storing return add in TEMP 2   
           "@5\n"
           "D=D-A\n"
           "@R6\n"
           "M=D\n"
        // Popping return value
           "@SP\n"
           "M=M-1\n"
           "A=M\n"
           "D=M\n"
           "@ARG\n"
           "A=M\n"
           "M=D\n"
        // Setting SP = ARG+1
           "@ARG\n"
           "D=M\n"
           "@SP\n"
           "M=D+1\n"
        // Setting THAT
            "@R5\n"
            "D=M\n"
            "D=D-1\n"
            "A=D\n"
            "D=M\n"
            "@THAT\n"
            "M=D\n"
        // Setting THIS
            "@R5\n"
            "D=M\n"
            "D=D-1\n"
            "D=D-1\n"
            "A=D\n"
            "D=M\n"
            "@THIS\n"
            "M=D\n"
        // Setting ARG
            "@R5\n"
            "D=M\n"
            "D=D-1\n"
            "D=D-1\n"
            "D=D-1\n"
            "A=D\n"
            "D=M\n"
            "@ARG\n"
            "M=D\n"
        // Setting LCL
            "@R5\n"
            "D=M\n"
            "D=D-1\n"
            "D=D-1\n"
            "D=D-1\n"
            "D=D-1\n"
            "A=D\n"
            "D=M\n"
            "@LCL\n"
            "M=D\n"
        // goto return
            "@R6\n"
            "A=M\n"
            "A=M\n"
            "0;JMP\n";


    return out;
           
}

int main() {
    unordered_map<string,int> aCommandCount = countInit();
    string file_name;
    cin >> file_name;
    fstream instr, out;
    instr.open(file_name, ios::in);
    out.open(outputName(file_name)+".asm", ios::out);

    if (instr.is_open()) {
        string line;
        bool inFunction = false;
        string fName;
        int nCalls = 0;
        while (getline(instr,line)) {

            if(line[0] == 0 || (line[0] == '/' && line[1] == '/') || line[0] == '(' ) {
                continue;
            }
            line = removeWhite(line);
            /*cout << line << endl;
            cout << commandType(line) << endl;*/
            string asmLine;
            string cType = commandType(line);
            string label;
            if (inFunction) {
                label = fName + "$" + arg1(line);
            } else {
                label = arg1(line);
            }

            if (cType == "C_ARITHMETIC") {
                asmLine = writeArithmetic(arg1(line),&aCommandCount);
            } else if (cType == "C_PUSH" || cType == "C_POP") {
                asmLine = writePushPop(file_name,cType,arg1(line),arg2(line));
            } else if (cType == "C_LABEL") {
                asmLine = writeLabel(label);
            } else if (cType == "C_GOTO") {
                asmLine = writeGoto(label);
            } else if (cType == "C_IF") {
                asmLine = writeIfGoto(label); 
            } else if (cType == "C_FUNCTION") {
                fName = funcName(file_name,arg1(line));
                inFunction = true;
                asmLine = writeFunction(fName,arg2(line));
            } else if (cType == "C_CALL") {
                //cout << "test 2------------------------" << endl;
                asmLine = writeCall(funcName(file_name,arg1(line)), arg2(line), nCalls);
                
                nCalls++;
            } else if (cType == "C_RETURN") {
                asmLine = writeReturn();
                nCalls = 0;
                inFunction = false;
            }
            out << asmLine << endl;
        }
        instr.close();
        out.close();
    }

}
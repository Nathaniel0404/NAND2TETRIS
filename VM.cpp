#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <format>

using namespace std;
using umap = unordered_map<string,string>;

umap commandList() {
    umap out;
    out["push"] = "C_PUSH";
    out["pop"] = "C_POP";

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
        if ((arg != "constant" && arg != "static") && arg != "pointer") {
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

string pop(string fileName, string segment, int index) {
    string out = "// pop " + segment + " " + to_string(index) + "\n";
    out += "@SP\nM=M-1\nA=M\nD=M\n";
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
    return name + ".asm";
}

int main() {
    unordered_map<string,int> aCommandCount = countInit();
    string file_name;
    cin >> file_name;
    fstream instr, out;
    instr.open(file_name, ios::in);
    out.open(outputName(file_name), ios::out);
    if (instr.is_open()) {
        string line;
        while (getline(instr,line)) {

            if(line[0] == 0 || (line[0] == '/' && line[1] == '/') || line[0] == '(' ) {
                continue;
            }
            string asmLine;
            string cType = commandType(line);
            if (cType == "C_ARITHMETIC") {
                asmLine = writeArithmetic(arg1(line),&aCommandCount);
            } else if (cType == "C_PUSH" || cType == "C_POP") {
                asmLine = writePushPop(file_name,cType,arg1(line),arg2(line));
            }
            out << asmLine << endl;
        }
        instr.close();
        out.close();
    }

}
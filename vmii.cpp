#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <format>
#include <typeinfo>
#include <filesystem>
using namespace std::filesystem;

using namespace std;
using umap = unordered_map<string,string>;

string dirToFile(string dir) {
    while (dir.find("/") < dir.length()) {      
        dir.erase(0,dir.find("/")+1);
    }

    return dir;
}

unordered_map<string,int> countInit() {
    unordered_map<string,int> out;
    out["eq"] = 0;
    out["gt"] = 0;
    out["lt"] = 0;

    return out;
}

string lineCleaner(string line) {
    if (line.find("//") < line.length()) {
        line.erase(line.find("//"),line.length());
    }
    while (line[0] == ' ' || line[0] == 9) {
        line.erase(0,1);
    }
    return line;
}

vector<string> lineParser(string line) {
    vector<string> set = {"","",""};

    for (int i =0; i < 3; i++) {
        if (line.length() == 0) {
            set[i] = "null";
        } else {
            while(line.length() != 0 && line[0] != ' ') {
                set[i] += line[0];
                line.erase(0,1);
            }
            line = lineCleaner(line);
        }
        
    }
    return set;
}

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
    out["temp"] = "R5";

    return out;
}

string commandType(string command) {
    umap commands = commandList();
    return commands[command];
}

vector<string> parser(string line, string fileName) {
    umap segMap = segmentList();
    vector<string> words = lineParser(line);
    string cType = commandType(words[0]);
    string arg1;
    string arg2 = words[2];
    if (cType == "C_ARITHMETIC") {
        arg1 = words[0];
    } else if (cType == "C_RETURN") {
        arg1 = "null";
    } else {
        if (segMap.find(words[1]) != segMap.end()) {
            arg1 = segMap[words[1]];
        } else {
            arg1 = words[1];
        }
    }
    
    return {cType,arg1,arg2};
}

//Helper functions
string accessSeg(string arg1, string arg2, string fileName) {
    string out;
    if (arg1 == "pointer") {
        if (arg2 == "0") {
            out += "@THIS\n";
        } else {
            out += "@THAT\n";
        }
    } else if (arg1 == "static") {
            out += "@" + fileName + "." + arg2 + "\n";
    } else {
        out += "@" + arg1 + "\n";
        if (arg1 != "R5") {
            out += "A=M\n";
        }
        for (int i = 0; i < stoi(arg2); i++) {
            out += "A=A+1\n";
        }
               
    }
    return out;
}

string addConstant(int n) {
    bool isNeg = false;
    if (n < 0) {
        n *= -1;
        isNeg = true;
    }
    string con = to_string(n);
    string out = "@" + con + "\n" ;
    if (isNeg) {
        out += "A=-A\n";
    }
    out += "D=A\n";
    return out;
}

//access stack pointer
string accessSP(bool checkLastValue) {
    string out = "@SP\n";
    if (checkLastValue) {
        out += "M=M-1\n";
    }
    out += "A=M\n";
    return out;
}

string increaseSP() {
    return "@SP\nM=M+1\n";
}

string resetSP() {
    string out = "@0\nD=A\n" + accessSP(false) + "M=D\n";

    return out;
}

string push(string arg1, string arg2, string fileName) {
    string out;
    if (arg1 == "constant") {
        out += "@" + arg2 + "\nD=A\n";
    } else {
        out += accessSeg(arg1, arg2,fileName) + "D=M\n";
    }
    out += accessSP(false) + "M=D\n" + increaseSP();
    return out;
}

string pop(string arg1, string arg2, string fileName) {
    string out;
    out += accessSP(true) + "D=M\n" + accessSeg(arg1,arg2,fileName) + "M=D\n" + resetSP();
    return out;
}

string pushPointer(string pointer) {
    string out = "@" + pointer + "\n" + "D=M\n" + accessSP(false) + "M=D\n" + increaseSP();

    return out;
}

string add() {
    string out;
    out += accessSP(true) + "D=M\n" + accessSP(true) + "M=D+M\n" + increaseSP();
    return out;
}

string sub() {
    string out;
    out += accessSP(true) + "D=M\n" + accessSP(true) + "M=M-D\n" + increaseSP();
    return out; 

}

string neg() {
    return accessSP(true) + "M=-M\n" + increaseSP();
}

string eq(int count) {
    string out;
    string EQ = "EQ" + to_string(count);
    out += accessSP(true) + "D=M\n" + accessSP(true) + "M=D-M\nD=M\n";
    out += "@" + EQ + "\nD;JEQ\n" + addConstant(0)
             + "@END" + EQ + "\n0;JMP\n" + "(" + EQ + ")\n" + addConstant(-1) + "(END" + EQ + ")\n"
             + accessSP(false) + "M=D\n" + increaseSP();
    return out;
}

string gt(int count) {
    string out;
    string GT = "GT" + to_string(count);
    out += accessSP(true) + "D=M\n" + accessSP(true) + "M=M-D\nD=M\n";
    out += "@" + GT + "\nD;JGT\n" + addConstant(0)
             + "@END" + GT + "\n0;JMP\n" + "(" + GT + ")\n" + addConstant(-1) + "(END" + GT + ")\n"
             + accessSP(false) + "M=D\n" + increaseSP();
    return out;
}

string lt(int count) {
    string out;
    string LT = "LT" + to_string(count);
    out += accessSP(true) + "D=M\n" + accessSP(true) + "M=M-D\nD=M\n";
    out += "@" + LT + "\nD;JLT\n" + addConstant(0)
             + "@END" + LT + "\n0;JMP\n" + "(" + LT + ")\n" + addConstant(-1) + "(END" + LT + ")\n"
             + accessSP(false) + "M=D\n" + increaseSP();
    return out;
}

string andC() {
    return accessSP(true) + "D=M\n" + accessSP(true) + "M=D&M\n" + increaseSP();
}

string orC() {
    return accessSP(true) + "D=M\n" + accessSP(true) + "M=D|M\n" + increaseSP();
}
string notC() {
    return accessSP(true) + "M=!M\n" + increaseSP();
}

string writeArithmetic(string arg1, unordered_map<string,int> &counter) {
    if (arg1 == "add") {
        return add();
    }
    if (arg1 == "sub") {
        return sub();
    }
    if (arg1 == "neg") {
        return neg();
    }
    if (arg1 == "eq") {
        counter[arg1]++;
        return eq(counter[arg1]);
    }
    if (arg1 == "gt") {
        counter[arg1]++;
        return gt(counter[arg1]);
    }
    if (arg1 == "lt") {
        counter[arg1]++;
        return lt(counter[arg1]);
    }
    if (arg1 == "and") {
        return andC();
    }
    if (arg1 == "or") {
        return orC();
    }

    return notC();
    
}

string functionNameGen(string file, string function) {
    return file + "." + function;
}

string labelNameGen(string file, string function, string label) {
    return file + "." + function + "$" + label;
}

string bracket(string label) {
    return "(" + label + ")";
}

string writeLabel(string file, string function, string label) {
    return bracket(labelNameGen(file, function, label));
}

string writeGOTO(string label) {
    return "@" + label + "\n0;JMP\n";
}

string writeIF(string file, string function, string label) {
    return accessSP(true) + "D=M\n" + "@"+ labelNameGen(file, function, label) + "\n0;JNE\n";
}

string writeFunction(string file, string function, string nVars) {
    int n = stoi(nVars);
    string out = bracket(functionNameGen(file, function)) + "\n";
    for (int i = 0; i < n; i++) {
        out += push("constant", "0", file);
    }
    return out;

}

string retAddGen(string file, string caller, int nCalls) {
    string label = "ret." + to_string(nCalls);
    return labelNameGen(file, caller, label);
}

string setPointerValue(string setter, string getter) {
    string out = "@" + getter + "\n" + "D=M\n";
    out += "@" + setter + "\n" + "M=D\n";
    return out;
}

string writeCall(string file, string caller, string callee, string nArgs, int nCalls) {
    int n = stoi(nArgs);
    string returnAdd = retAddGen(file, caller, nCalls);
    string out;
    out += "/// Push return address\n";
    out += push("constant", returnAdd, file);
    out += "/// Push mem segments\n";
    out += pushPointer("LCL");
    out += pushPointer("ARG");
    out += pushPointer("THIS");
    out += pushPointer("THAT");
    out += "/// Reposition ARG=SP-5-nARG\n";
    out += "@SP\n";
    for (int i = 0; i < 5 + n; i++) {
        out += "A=A-1\n";
    }
    out += "D=A\n";
    out += "@ARG\nM=D\n";
    out += "/// Reposition LCL=SP\n";
    out += setPointerValue("LCL", "SP");
    out += "/// Goto function\n";
    out += writeGOTO(functionNameGen(file,callee));
    out += bracket(returnAdd);
    return out;
}

string resetSeg(string segment, int index) {
    string out = "@R5\nA=M\n";
    for (int i = 0; i < index; i++) {
        out += "A=A-1\n";
    }
    out += "@" + segment + "\nM=D\n";
}

string writeReturn(string file) {
    // Save frame in a temp variable
    string out = "/// Save frame in a temp variable\n";
    out += "@LCL\nD=M\n";
    out += accessSeg("R5", "0", file) + "M=D";
    // Save return address in another temp variable
    out += "/// Save return address in another temp variable\n";
    out += "@LCL\nA=M\n";
    for (int i = 0; i < 5; i++) {
        out += "A=A-1\n";
    }
    out  += "D=M\n";
    out += accessSeg("R5","1",file) + "M=D\n";
    // Pop return value to top of stack
    out += "/// Pop return value to top of stack\n";
    out += accessSP(true) + "D=M\n";
    out += "@ARG\nA=M\nM=D\n";
    // Reset SP to after return value
    out += "/// Reset SP to after return value\n";
    out  += "@ARG\nD=M\n@SP\nM=D+1\n";
    // Reset memory segments
    out += "/// Reset memory segments\n";
    out += resetSeg("THAT",1);
    out += resetSeg("THIS",2);
    out += resetSeg("ARG",3);
    out += resetSeg("LCL",4);
    // Go to return address
    out += "/// Go to return address\n";
    out += accessSeg("R5","1",file) + "A=M\n0;JMP\n";
    return out;
}

//code Writer
string codeWriter(string line, string fileName, unordered_map<string,int> &counter) {
    string asmLine = "//" + line + "\n";
    vector<string> keywords = parser(line,fileName);
    string cType = keywords[0];
    string arg1 = keywords[1];
    string arg2 = keywords[2];
    
    if (cType == "C_PUSH") {
        asmLine += push(arg1,arg2,fileName);
    } else if (cType == "C_POP") {
        asmLine += pop(arg1,arg2,fileName);
    } else if (cType == "C_ARITHMETIC") {
        asmLine += writeArithmetic(arg1,counter);
    }


    return asmLine;
}

int main() {

    path directory;
    fstream instruction, out;
    string fileT = "test";
    string funcT = "foo";
    cout << "write function\n" << writeFunction(fileT,funcT,"3") << endl;
    cout << "write call\n" << writeCall(fileT,"bar", funcT, "3", 2) << endl;
    cout << "write return\n" << writeReturn(fileT) << endl;
    cout << "Enter a path: " << endl;
    cin >> directory;
    string dirString = directory.string();
    out.open(dirToFile(dirString)+".asm",ios::out);

    unordered_map<string,int> cCounter = countInit();

    for (const auto& file : directory_iterator(directory)) {
        path filePath = file.path();
        string sfile = filePath.string();
        if (sfile.find(".vm") < sfile.length()) {
            instruction.open(sfile,ios::in);
            string line;
            if (instruction.is_open()) {
            while (getline(instruction,line)) {
                line = lineCleaner(line);
                if (line.length() == 0) {
                    continue;
                }
                out << codeWriter(line, dirToFile(sfile), cCounter) << endl;
            }
            instruction.close();
            }
        }
    }
    out.close();
}
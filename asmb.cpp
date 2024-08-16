#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <unordered_map>

using namespace std;
using st = unordered_map<string,string>;

string intToBin(string line) {
    int add = stoi(line);
    string bin = "0000000000000000";
    for (int i = 1; i < 16; i++) {
        int posValue = pow(2,15 - i);
        if (add >= posValue) {
            bin[i] = '1';
            add -= posValue;
        }
    }
    return bin;
}

void initSymb(st *comp, st *dest, st *jmp) {
    //Dest symbols
    (*dest)["null"] = "000";
    (*dest)["M"] = "001";
    (*dest)["D"] = "010";
    (*dest)["DM"] = "011";
    (*dest)["MD"] = "011";
    (*dest)["A"] = "100";
    (*dest)["AM"] = "101";
    (*dest)["MA"] = "101";
    (*dest)["AD"] = "110";
    (*dest)["DA"] = "110";
    (*dest)["ADM"] = "111";
    //Predef symbols
    (*dest)["R0"] = "0";
    (*dest)["R1"] = "1";
    (*dest)["R2"] = "2";
    (*dest)["R3"] = "3";
    (*dest)["R4"] = "4";
    (*dest)["R5"] = "5";
    (*dest)["R6"] = "6";
    (*dest)["R7"] = "7";
    (*dest)["R8"] = "8";
    (*dest)["R9"] = "9";
    (*dest)["R10"] = "10";
    (*dest)["R11"] = "11";
    (*dest)["R12"] = "12";
    (*dest)["R13"] = "13";
    (*dest)["R14"] = "14";
    (*dest)["R15"] = "15";
    (*dest)["SP"] = "0";
    (*dest)["LCL"] = "1";
    (*dest)["ARG"] = "2";
    (*dest)["THIS"] = "3";
    (*dest)["THAT"] = "4";
    (*dest)["SCREEN"] = "16384";
    (*dest)["KBD"] = "24576";

    //Comp symbols; a == 0
    (*comp)["0"] = "0101010";
    (*comp)["1"] = "0111111";
    (*comp)["-1"] = "0111010";
    (*comp)["D"] = "0001100";
    (*comp)["A"] = "0110000";
    (*comp)["!D"] = "0001101";
    (*comp)["!A"] = "0110001";
    (*comp)["-D"] = "0001111";
    (*comp)["-A"] = "0110011";
    (*comp)["D+1"] = "0011111";
    (*comp)["A+1"] = "0110111";
    (*comp)["D-1"] = "0001110";
    (*comp)["A-1"] = "0110010";
    (*comp)["D+A"] = "0000010";
    (*comp)["D-A"] = "0010011";
    (*comp)["A-D"] = "0000111";
    (*comp)["D&A"] = "0000000";
    (*comp)["D|A"] = "0010101";
    //Comp symbols; a = 1
    (*comp)["M"] = "1110000";
    (*comp)["!M"] = "1110001";
    (*comp)["-M"] = "1110011";
    (*comp)["M+1"] = "1110111";
    (*comp)["M-1"] = "1110010";
    (*comp)["D+M"] = "1000010";
    (*comp)["D-M"] = "1010011";
    (*comp)["M-D"] = "1000111";
    (*comp)["D&M"] = "1000000";
    (*comp)["D|M"] = "1010101";
    //Jmp symbols
    (*jmp)["null"] = "000";
    (*jmp)["JGT"] = "001";
    (*jmp)["JEQ"] = "010";
    (*jmp)["JGE"] = "011";
    (*jmp)["JLT"] = "100";
    (*jmp)["JNE"] = "101";
    (*jmp)["JLE"] = "110";
    (*jmp)["JMP"] = "111";
}

string cComp(string line, st symb_comp, bool hasDest) {
    string comp;
    if (hasDest) {
        bool start = false;
        for (char c : line) {
            if (c == '=') {
                start = true;
                continue;
            } else if (c == ';' || c == '/') {
                break;
            } else if (start) {
                comp += c;
            }
        }
    } else {
        for (char c : line) {
            if (c == ';' || c == '/') {
                break;
            }
            comp += c;
        }
    }
    return symb_comp[comp];
}

string cDest(string line, st symb_dest, bool hasDest) {
    string dest;
    if (hasDest) {
        for (char c : line) {
            if (c == '=') {
                break;
            } else {
                dest += c;
            }
        }
    } else {
        dest = "null";
    }
    return symb_dest[dest];
}

string cJmp(string line, st symb_jmp, bool hasJmp) {
    string jmp;
    if (hasJmp) {
        bool start = false;
        for (char c : line) {
            if (c == '/') {
                break;
            }
            if (c == ';') {
                start = true;
                continue;
            } else if (start) {
                jmp += c;
            }
        }
    } else {
        jmp = "null";
    }
    return symb_jmp[jmp];
}

string c_instruction(string line, st symb_comp, st symb_dest, st symb_jmp) {
    bool hasDest = false;
    bool hasJmp = false;
    for (char c : line) {
        if (c == '=') {
            hasDest = true;
        } else if ( c == ';') {
            hasJmp = true;
        }
    }
    
    string init = "111";
    string comp = cComp(line, symb_comp, hasDest);
    string dest = cDest(line,symb_dest,hasDest);
    string jmp = cJmp(line,symb_jmp,hasJmp);
    return init + comp + dest + jmp;
}



string removeSpace (string line) {
    string newLine;
    for (char c : line) {
        if (c != ' ') {
            newLine += c;
        }
    }
    return newLine;
}

string getLabel (string line) {
    string label;
    for (int i = 1; i < line.length(); i++) {
        if (line[i] == ')') {
            break;
        } else {
            label += line[i];
        }
    }
    return label;
}

string outputName (string file) {
    string name;
    for (char c : file) {
        if (c == '.') {
            break;
        }
        name += c;
    }
    return name + ".hack";
}

int main() {
    string file_name;
    cin >> file_name;
    
    st symb_comp, symb_dest, symb_jmp;
    initSymb(&symb_comp,&symb_dest,&symb_jmp);

    fstream instr, out;
    instr.open(file_name, ios::in);

    //First Parse
    if (instr.is_open()) {
        string line;
        int line_n = -1;
        while (getline(instr, line)) {
            line = removeSpace(line);
            if(line[0] == 0 || (line[0] == '/' && line[1] == '/')) {
                continue;
            } else if (line[0] == '(') {
                string label = getLabel(line);
                symb_dest[label] = to_string(line_n + 1);
            } else {
                line_n++;
            }
        }
        instr.close();
    }
    
    //Second Parse
    instr.open(file_name, ios::in);
    out.open(outputName(file_name), ios::out);
    if (instr.is_open() && out.is_open()) {
        string line;
        string bin = "0000000000000000";
        int var = 16;
        while (getline(instr, line)) {
            line = removeSpace(line);
            if(line[0] == 0 || (line[0] == '/' && line[1] == '/') || line[0] == '(' ) {
                continue;
            }   
            if (line[0] == '@') {
                line.erase(0,1);
                if (line[0] >= '0' && line[0] <= '9') {
                    bin = intToBin(line);
                } else {
                    if (symb_dest.find(line) != symb_dest.end()) {
                        bin = intToBin(symb_dest[line]);
                    } else {
                        symb_dest[line] = to_string(var);
                        bin = intToBin(to_string(var));
                        var++;
                    }
                }
                out << bin << endl;
            } else  {
                line = c_instruction(line,symb_comp,symb_dest,symb_jmp);
                out << line << endl;
                
            }
        }
        instr.close();
        out.close();
    }


}
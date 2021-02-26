#include <iostream>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef struct Var{
    string VarName;
    vector<string> value;
}VarNode;

typedef struct Function{
    string FuncName;
    string attr;
    vector<string> para;
    vector<VarNode> Var;
}FunctionNode;

typedef struct Class{
  string ClassName;
  string attr;
  string interface;
  vector<VarNode> Var;
  vector<FunctionNode> Function;
}ClassNode;

typedef struct Interface{
  string InterfaceName;
  string attr;
  vector<VarNode> Var;
  vector<FunctionNode> Function;
}InterfaceNode;

typedef struct file{
    string FileName;
    vector<string> Dependence;
    vector<ClassNode> Class;
    vector<InterfaceNode> Interface;
    vector<int> child;
    string space;
}FileNode;

typedef struct FlagNode{
    string FileName;
    string ClassName;
    string FunctionName;
    string VarName;
    int FileNo;
    string value;
}Flagnode;

void SetFlagStr();
void GetAllFile(const string& path);
void HandleFile();
int CheckNote(const string& Parse);
int CheckUsing(const string& Parse, int FileNo);
int CheckSpace(const string& Parse, int FileNo);
void CheckClassName(const string& Parse, int FileNo);
void CheckClassElement(const string& Parse, int FileNo);
void GetRelation();
void FindFlagStr();
int CheckFlag(VarNode var);

void GetFinalResult();
int ChecktempStr(string Str);
string FindStr(string str, int FileNo, string FuncName);

void Debug();

vector<string> Flagstr;
vector<FileNode> FileList;
vector<Flagnode> FlagNodeList;

int classPart = 0;
int no;
string functionName;

int main() {
    string FILEPATH = "../cartservice/";
    GetAllFile(FILEPATH);
    SetFlagStr();
    HandleFile();

    GetRelation();
    FindFlagStr();

    GetFinalResult();

    //Debug();
    return 0;
}

void GetAllFile(const string& Path)
{
    struct dirent *dirp;

    DIR* dir = opendir(Path.c_str());

    while ((dirp = readdir(dir)) != nullptr) {
        if((!strncmp(dirp->d_name, ".", 1)) || (!strncmp(dirp->d_name, "..", 2)))
            continue;

        if (dirp->d_type == DT_REG) {
            if(strstr(dirp->d_name, ".cs") != nullptr && strstr(dirp->d_name, ".csp") == nullptr){
                string tempPath = Path;
                tempPath.append(dirp->d_name);
                FileNode file;
                file.FileName = tempPath;
                FileList.push_back(file);
            }
        }
        else if(dirp->d_type == DT_DIR) {
            string tempPath = Path;
            tempPath.append(dirp->d_name);
            tempPath.append("/");
            GetAllFile(tempPath);
        }
    }

    closedir(dir);
}

void SetFlagStr()
{
    //db.Ping, db.HashSetAsync, db.HashGetAsync, db.StringSet, db.StringGet
    /*Flagstr.emplace_back("db.Ping");
    Flagstr.emplace_back("db.HashSetAsync");
    Flagstr.emplace_back("db.HashGetAsync");
    Flagstr.emplace_back("db.StringSet");
    Flagstr.emplace_back("db.StringGet");*/
    Flagstr.emplace_back("ConnectionMultiplexer.Connect");
}

void HandleFile()
{
    for(int i=0; i<FileList.size(); i++){
        string Parse;
        ifstream File;

        int usingPart = 0;
        int spacePart = 1;
        int functionPart = 0;

        File.open(FileList[i].FileName, ios::binary);

        if(!File){
            cout << "Can't Find File." << endl;
            return;
        }

        while(getline(File, Parse)){
            if(CheckNote(Parse) == 0){
                continue;
            }
            else{
                if(usingPart == 0){
                    usingPart = CheckUsing(Parse, i);
                }
                if(spacePart != 0){
                    spacePart = CheckSpace(Parse, i);
                }
                if(classPart == 0 && spacePart == 0 && usingPart != 0){
                    CheckClassName(Parse, i);
                }
                else{
                    CheckClassElement(Parse, i);
                }
            }
        }

        usingPart = 0;
        spacePart = 1;
        functionPart = 0;
        classPart = 0;
    }
}

int CheckNote(const string& Parse)
{
    int tempPos = 0;
    while(tempPos < Parse.length() && Parse[tempPos] == ' '){
        tempPos++;
    }
    if((Parse[tempPos] == '/' && Parse[tempPos+1] == '/') || tempPos == Parse.length()){
        return 0;
    }
    else{
        return 1;
    }
}
int CheckUsing(const string& Parse, int FileNo)
{
    string flag = "using";
    string::size_type pos = Parse.find(flag);
    if(pos != std::string::npos && Parse[(int)pos+flag.length()] == ' '){
        int tempPos = (int)pos + flag.length();
        while(Parse[tempPos] == ' '){
            tempPos++;
        }
        string result = Parse.substr(tempPos, Parse.length()-tempPos-1);
        FileList[FileNo].Dependence.push_back(result);
        return 0;
    }
    else{
        return 1;
    }
}
int CheckSpace(const string& Parse, int FileNo)
{
    string flag = "namespace";
    string::size_type pos = Parse.find(flag);
    if(pos != std::string::npos && Parse[(int)pos+flag.length()] == ' '){
        int tempPos = (int)pos + flag.length();
        while(Parse[tempPos] == ' '){
            tempPos++;
        }
        FileList[FileNo].space = Parse.substr(tempPos, Parse.length()-tempPos);
        return 0;
    }
    else{
        return 1;
    }
}
void CheckClassName(const string& Parse, int FileNo)
{
    string flagClass = "class";
    string flagInterface = "interface";
    string::size_type posClass = Parse.find(flagClass);
    string::size_type posInterface = Parse.find(flagInterface);
    if(posClass != std::string::npos && Parse[(int)posClass+flagClass.length()] == ' '){
        int beginPos, endPos;
        ClassNode tempClass;
        beginPos = 0;
        while(Parse[beginPos] == ' '){
            beginPos++;
        }
        endPos = (int)posClass - 1;
        while(Parse[endPos] == ' '){
            endPos--;
        }
        tempClass.attr = Parse.substr(beginPos, endPos-beginPos+1);

        int tempPos = (int)posClass + flagClass.length();
        while(Parse[tempPos] == ' '){
            tempPos++;
        }
        beginPos = tempPos;
        while(Parse[tempPos] != ' ' && tempPos < Parse.length()){
            tempPos++;
        }
        endPos = tempPos;
        tempClass.ClassName = Parse.substr(beginPos, endPos-beginPos);
        if(endPos != Parse.length()){
            while(Parse[tempPos] == ' '){
                tempPos++;
            }
            if(Parse[tempPos] == ':'){
                tempPos++;
            }
            while(Parse[tempPos] == ' '){
                tempPos++;
            }
            tempClass.interface = Parse.substr(tempPos, Parse.length()-tempPos);
        }
        classPart++;
        FileList[FileNo].Class.push_back(tempClass);
    }
    else if(posInterface != std::string::npos && Parse[(int)posInterface+flagInterface.length()] == ' '){
        int tempPos = (int)posInterface + flagInterface.length();
        while(Parse[tempPos] == ' '){
            tempPos++;
        }
        InterfaceNode tempClass;
        tempClass.InterfaceName = Parse.substr(tempPos, Parse.length()-tempPos);
        int beginPos, endPos;
        beginPos = 0;
        while(Parse[beginPos] == ' '){
            beginPos++;
        }
        endPos = (int)posInterface - 1;
        while(Parse[endPos] == ' '){
            endPos--;
        }
        tempClass.attr = Parse.substr(beginPos, endPos-beginPos+1);
        classPart++;
        FileList[FileNo].Interface.push_back(tempClass);
    }
    else{
        return;
    }
}
void CheckClassElement(const string& Parse, int FileNo)
{
    string::size_type pos;
    pos = Parse.find('{');
    if(((int)pos+1 == Parse.length() || Parse[pos+1] == ' ')&& pos != std::string::npos){
        classPart++;
    }

    string FlagFunc[2] = {"public", "private"};
    FunctionNode tempFunc;

    for(int i=0; i<2; i++){
        pos = Parse.find(FlagFunc[i]);
        string::size_type checkpos = Parse.find(';');
        // function define
        if(pos != std::string::npos && checkpos == std::string::npos && Parse[pos-1] == ' ' && Parse[pos+FlagFunc[i].length()] == ' '){
            int beginPos, endPos;
            tempFunc.attr = FlagFunc[i];
            int tempPos = (int)pos + FlagFunc[i].length();
            while(Parse[tempPos] == ' '){
                tempPos++;
            }
            beginPos = tempPos;
            while(Parse[tempPos] != '('){
                tempPos++;
            }
            endPos = tempPos;
            tempFunc.FuncName = Parse.substr(beginPos, endPos-beginPos);
            tempPos++;
            while(Parse[tempPos] != ')'){
                while(Parse[tempPos] == ' '){
                    tempPos++;
                }
                while(Parse[tempPos] != ' '){
                    tempPos++;
                }
                while(Parse[tempPos] == ' '){
                    tempPos++;
                }
                beginPos = tempPos;
                while(Parse[tempPos] != ',' && Parse[tempPos] != ')'){
                    tempPos++;
                }
                endPos = tempPos;
                tempFunc.para.push_back(Parse.substr(beginPos, endPos-beginPos));
                if(Parse[tempPos] == ','){
                    tempPos++;
                }
            }
        }
    }

    pos = Parse.find('=');
    if(pos != std::string::npos && Parse[pos-1] == ' ' && Parse[pos+1] == ' '){
        VarNode tempVar;
        int beginPos, endPos;
        int tempPos = (int)pos;
          tempPos--;
          while(Parse[tempPos] == ' '){
              tempPos--;
          }
          endPos = tempPos;
          while(Parse[tempPos] != ' '){
              tempPos--;
          }
          beginPos = tempPos;
          string tempName = Parse.substr(beginPos+1, endPos-beginPos);
          tempPos = (int)pos + 1;
          while(Parse[tempPos] == ' '){
              tempPos++;
          }
          beginPos = tempPos;
          while(Parse[tempPos] != ';' && tempPos < Parse.length()){
              tempPos++;
          }
          endPos = tempPos;
          string tempValue = Parse.substr(beginPos, endPos-beginPos);
          int exist = 0;
          for(int i=0; i<FileList[FileNo].Class[0].Var.size(); i++){
              if((int)FileList[FileNo].Class[0].Var[i].VarName.find(tempName) == 0 && tempName.length() == FileList[FileNo].Class[0].Var[i].VarName.length()){
                  exist = 1;
                  FileList[FileNo].Class[0].Var[i].value.push_back(tempValue);
              }
          }
          if(exist == 0 && classPart>2){
              for(int i=0; i<FileList[FileNo].Class[0].Function[FileList[FileNo].Class[0].Function.size()-1].Var.size(); i++){
                  if((int)FileList[FileNo].Class[0].Function[FileList[FileNo].Class[0].Function.size()-1].Var[i].VarName.find(tempName) == 0 && tempName.length() == FileList[FileNo].Class[0].Function[FileList[FileNo].Class[0].Function.size()-1].Var[i].VarName.length()){
                      exist = 1;
                      FileList[FileNo].Class[0].Function[FileList[FileNo].Class[0].Function.size()-1].Var[i].value.push_back(tempValue);
                  }
              }
          }

          if(exist == 0){
              tempVar.value.push_back(tempValue);
              tempVar.VarName = tempName;
              if(classPart == 2){
                  FileList[FileNo].Class[0].Var.push_back(tempVar);
              }
              else{
                  FileList[FileNo].Class[0].Function[FileList[FileNo].Class[0].Function.size()-1].Var.push_back(tempVar);
              }
          }
    }

    if(classPart == 2 && tempFunc.FuncName.length()!=0){
        FileList[FileNo].Class[FileList[FileNo].Class.size()-1].Function.push_back(tempFunc);
    }

    pos = Parse.find('}');
    if(((int)pos+1 == Parse.length() || Parse[pos-1] == ' ' || Parse[pos+1] == ')') && pos != std::string::npos){
        classPart--;
    }
}

void GetRelation()
{
    string::size_type pos;
    for(int i=0; i<FileList.size(); i++){
        for(int j=i+1; j<FileList.size(); j++){
            for(int k=0; k<FileList[j].Dependence.size(); k++){
                pos = FileList[j].Dependence[k].find(FileList[i].space);
                if(pos != std::string::npos){
                    FileList[i].child.push_back(j);
                }
            }
        }
    }
}

void FindFlagStr()
{
    int getflag = 0;
    for(int i=0; i<FileList.size(); i++){
        for(int j=0; j<FileList[i].Class.size(); j++){
            for(int k=0; k<FileList[i].Class[j].Var.size(); k++){
                getflag = CheckFlag(FileList[i].Class[j].Var[k]);
                if(getflag != 0){
                    Flagnode flagnode;
                    flagnode.FileName = FileList[i].FileName;
                    flagnode.FileNo = i;
                    flagnode.ClassName = FileList[i].Class[j].ClassName;
                    flagnode.FunctionName = "NULL";
                    flagnode.VarName = FileList[i].Class[j].Var[k].VarName;
                    flagnode.value = FileList[i].Class[j].Var[k].value[getflag-1];
                    FlagNodeList.push_back(flagnode);
                }
            }

            for(int k=0; k<FileList[i].Class[j].Function.size(); k++){
                for(int m=0; m<FileList[i].Class[j].Function[k].Var.size(); m++){
                    getflag = CheckFlag(FileList[i].Class[j].Function[k].Var[m]);
                    if(getflag != 0){
                        Flagnode flagnode;
                        flagnode.FileName = FileList[i].FileName;
                        flagnode.FileNo = i;
                        flagnode.ClassName = FileList[i].Class[j].ClassName;
                        flagnode.FunctionName = FileList[i].Class[j].Function[k].FuncName;
                        flagnode.VarName = FileList[i].Class[j].Function[k].Var[m].VarName;
                        flagnode.value = FileList[i].Class[j].Function[k].Var[m].value[getflag-1];
                        FlagNodeList.push_back(flagnode);
                    }
                }
            }
        }
    }
}
int CheckFlag(VarNode var)
{
    string::size_type pos;
    for(int i=0; i<Flagstr.size(); i++){
        for(int j=0; j<var.value.size(); j++){
            pos = var.value[j].find(Flagstr[i]);
            if(pos != std::string::npos){
                return j+1;
            }
        }
    }

    return 0;
}

void GetFinalResult()
{
    for(int i=0; i<FlagNodeList.size(); i++){
        string result;
        int type = -1;

        string::size_type pos;
        pos = FlagNodeList[i].value.find('(');
        string tempStr;
        if(pos != std::string::npos){
            int beginPos, endPos;
            int tempPos = (int)pos + 1;
            beginPos = tempPos;
            while(FlagNodeList[i].value[tempPos] != ')'){
                tempPos++;
            }
            endPos = tempPos;
            tempStr = FlagNodeList[i].value.substr(beginPos, endPos-beginPos);
            type = ChecktempStr(tempStr);
        }
        no = FlagNodeList[i].FileNo;
        functionName = FlagNodeList[i].FunctionName;

        int length, replacepos;

        while(type != 0){
            if(type == 1){
                pos = tempStr.find('"');
                int beginPos, endPos;
                beginPos = (int)pos + 1;
                pos++;
                while(tempStr[pos] != '"'){
                    pos++;
                }
                endPos = pos;
                result = tempStr.substr(beginPos, endPos-beginPos);

                pos = tempStr.find('{');
                replacepos = (int)pos - beginPos - 1;
                beginPos = (int)pos;
                pos++;
                while(tempStr[pos] != '}'){
                    pos++;
                }
                endPos = pos;
                tempStr = tempStr.substr(beginPos+1, endPos-beginPos-1);
                length = endPos-beginPos+1;
            }
            else if(type == 2){
                string temp = FindStr(tempStr, no, functionName);
                pos = temp.find('(');
                if(pos != std::string::npos){
                    int beginPos, endPos;
                    int tempPos = (int)pos + 1;
                    beginPos = tempPos;
                    while(temp[tempPos] != ')'){
                        tempPos++;
                    }
                    endPos = tempPos;
                    tempStr = temp.substr(beginPos, endPos-beginPos);
                }
                else{
                    tempStr = temp;
                }
            }

            type = ChecktempStr(tempStr);
        }

        pos = tempStr.find('"');
        if(pos != std::string::npos){
            int beginPos, endPos;
            int tempPos = (int)pos + 1;
            beginPos = tempPos;
            while(tempStr[tempPos] != '"'){
                tempPos++;
            }
            endPos = tempPos;
            tempStr = tempStr.substr(beginPos, endPos-beginPos);
        }

        string final;
        final = result.substr(0, replacepos+1);
        final.append(tempStr);
        final.append(result.substr(replacepos+length+1, result.length()-length-replacepos));
        cout << final << endl;
    }
}

int ChecktempStr(string str)
{
    string::size_type pos_syh, pos_dkh;
    pos_syh = str.find('"');
    pos_dkh = str.find('{');
    if(pos_syh != std::string::npos && pos_dkh == std::string::npos){
        return 0;
    }
    else if(pos_syh != std::string::npos && pos_dkh != std::string::npos){
        return 1;
    }
    else{
        return 2;
    }
}
string FindStr(string Str, int FileNo, string FuncName)
{
    string::size_type pos;
    FunctionNode tempFunc;
    int Check = 0;

    for(int j=0; j<FileList[FileNo].Class.size(); j++){
        for(int k=0; k<FileList[FileNo].Class[j].Var.size(); k++){
            pos = FileList[FileNo].Class[j].Var[k].VarName.find(Str);
            if(pos != std::string::npos && Str.length() == FileList[FileNo].Class[j].Var[k].VarName.length()){
                return FileList[FileNo].Class[j].Var[k].value[0];
            }
        }
        for(int k=0; k<FileList[FileNo].Class[j].Function.size(); k++){
            //if(FuncName == FileList[FileNo].Class[j].Function[k].FuncName){
                for(int m=0; m<FileList[FileNo].Class[j].Function[k].para.size(); m++){
                    if(FileList[FileNo].Class[j].Function[k].para[m] == Str){
                        tempFunc = FileList[FileNo].Class[j].Function[k];
                        Check = m+1;
                        break;
                    }
                }
                if(Check == 0){
                    for(int m=0; m<FileList[FileNo].Class[j].Function[k].Var.size(); m++){
                        if(FileList[FileNo].Class[j].Function[k].Var[m].VarName == Str){
                            return FileList[FileNo].Class[j].Function[k].Var[m].value[0];
                        }
                    }
                }
            //}
        }
    }

    if(Check != 0){
        for(int i=0; i<FileList[FileNo].child.size(); i++){
            for(int j=0; j<FileList[FileList[FileNo].child[i]].Class.size(); j++){
                ClassNode tempClass = FileList[FileList[FileNo].child[i]].Class[j];
                for(int k=0; k<tempClass.Var.size(); k++){
                    for(int m=0; m<tempClass.Var[k].value.size(); m++){
                        pos = tempClass.Var[k].value[m].find(tempFunc.FuncName);
                        if(pos != std::string::npos){
                            no = FileList[FileNo].child[i];
                            functionName = "";
                            return tempClass.Var[k].value[m];
                        }
                    }
                }
                for(int k=0; k<tempClass.Function.size(); k++){
                    for(int m=0; m<tempClass.Function[k].Var.size(); m++){
                        for(int n=0; n<tempClass.Function[k].Var[m].value.size(); n++){
                            pos = tempClass.Function[k].Var[m].value[n].find(tempFunc.FuncName);
                            if(pos != std::string::npos){
                                no = FileList[FileNo].child[i];
                                functionName = tempClass.Function[k].FuncName;
                                return tempClass.Function[k].Var[m].value[n];
                            }
                        }
                    }
                }
            }
        }
    }
}

void Debug()
{
    for(int i=0; i<FileList.size(); i++){
        cout << "File: " << FileList[i].FileName << endl;
        cout << "Using: " << endl;
        for(int j=0; j<FileList[i].Dependence.size(); j++){
            cout << "    " << FileList[i].Dependence[j] << endl;
        }
        cout << "namespace: " << FileList[i].space << endl;
        for(int j=0; j<FileList[i].Class.size(); j++){
            cout << "Class: " << endl;
            cout << "   ClassName: " << FileList[i].Class[j].ClassName << endl;
            cout << "   ClassAttr: " << FileList[i].Class[j].attr << endl;
            cout << "   ClassInterface: " << FileList[i].Class[j].interface << endl;
            for(int k=0; k<FileList[i].Class[j].Var.size(); k++){
                cout << "       VarName: " << FileList[i].Class[j].Var[k].VarName << endl;
                for(int m=0; m<FileList[i].Class[j].Var[k].value.size(); m++){
                    cout << "       VarValue: " << FileList[i].Class[j].Var[k].value[m] << endl;
                }
            }
            for(int k=0; k<FileList[i].Class[j].Function.size(); k++){
                cout << "       FunctionName: " << FileList[i].Class[j].Function[k].FuncName << endl;
                for(int m=0; m<FileList[i].Class[j].Function[k].para.size(); m++){
                    cout << "           FunctionPara: " << FileList[i].Class[j].Function[k].para[m] << endl;
                }
                for(int m=0; m<FileList[i].Class[j].Function[k].Var.size(); m++){
                    cout << "           FunctionVarName: " << FileList[i].Class[j].Function[k].Var[m].VarName << endl;
                    for(int n=0; n<FileList[i].Class[j].Function[k].Var[m].value.size(); n++){
                        cout << "           FunctionVarValue: " << FileList[i].Class[j].Function[k].Var[m].value[n] << endl;
                    }
                }
            }
        }
        for(int j=0; j<FileList[i].Interface.size(); j++){
            cout << "Interface: " << endl;
            cout << "   InterfaceName: " << FileList[i].Interface[j].InterfaceName << endl;
            cout << "   InterfaceAttr: " << FileList[i].Interface[j].attr << endl;
        }
        cout << endl;
    }
}

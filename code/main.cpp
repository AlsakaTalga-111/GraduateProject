#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef struct httpNode{
    string ModuleName;
    string method;
    vector<string> value;
    string object;
    int defNo;
}HttpValueNode;
vector<HttpValueNode> HttpValueList;

typedef struct DefModule{
    string name;
    vector<string> paras;
    vector<string> value;
    vector<string> id;
    vector<int> ParentMod;
}DefNode;
vector<DefNode> DefModuleList;

typedef struct stringnode{
    string value;
    int type;
}stringNode;

string FILE_PATH = "../SourceFile/ruby.rb";

vector<stringNode> CheckAllString(const string& RubyParse);
void CheckNetHttp();
void GetModule(const string& RubyParse);
void GetModuleValue(const string& RubyParse);
void GetValue();
string findVarValue(const string& str, int defNo);
void GetRelation();

void SetFlag();
vector<string> FlagStr;

// debug
void DebugHttpList();
void DebugDefModuleList();

void Print();

int DefModule = 0;
int DefNo = -1;

int main() {
    ifstream File;
    File.open(FILE_PATH, ios::binary);
    if(!File){
        cout << "Can't Find File." << endl;
        return 0;
    }

    string RubyParse;
    SetFlag();

    while(getline(File, RubyParse)){
        if(DefModule == 0){
            GetModule(RubyParse);
        }
        else{
            GetModuleValue(RubyParse);
        }
    }

    GetRelation();
    CheckNetHttp();
    GetValue();

    Print();

    File.close();

    return 0;
}

vector<stringNode> CheckAllString(const string& RubyParse)
{
    int BeginPos = -1;
    int EndPos = -1;
    int GetString = 0;
    int GetVar = 0;
    vector<stringNode> Str;
    stringNode tempstr;

    int tempPos = 0;
    while(tempPos < RubyParse.length()){
        if((RubyParse[tempPos] == '\"' || RubyParse[tempPos] == '\'') && GetString == 0 && GetVar == 0){
            BeginPos = tempPos;
            GetString = 1;
        }
        else if((RubyParse[tempPos] == '\"' || RubyParse[tempPos] == '\'') && GetString == 1 && GetVar == 0){
            EndPos = tempPos;
            GetString = 0;
            tempstr.value = RubyParse.substr(BeginPos+1, EndPos-BeginPos-1);
            tempstr.type = 0;
            Str.push_back(tempstr);
        }

        if(GetString == 0 && GetVar == 0 && RubyParse[tempPos] != ' ' && RubyParse[tempPos] != '+' && RubyParse[tempPos] != '\"' && RubyParse[tempPos] != '\''){
            BeginPos = tempPos;
            GetVar = 1;
        }
        else if(GetString == 0 && GetVar == 1 && (RubyParse[tempPos] == ' ' || tempPos == RubyParse.size()-1)){
            EndPos = tempPos;
            GetVar = 0;
            tempstr.value = RubyParse.substr(BeginPos, EndPos-BeginPos+1);
            tempstr.type = 1;
            Str.push_back(tempstr);
        }

        tempPos++;
    }

    /*for(const auto & i : Str){
        cout <<  i.type << " " << i.value << endl;
    }*/

    return Str;
}

void CheckNetHttp()
{
    for(int f=0; f<FlagStr.size(); f++){
        for(int i=0; i<DefModuleList.size(); i++) {
            for (int j = 0; j < DefModuleList[i].id.size(); j++) {
                string::size_type pos = DefModuleList[i].value[j].find(FlagStr[f]);
                if (pos != std::string::npos) {
                    // find it
                    // Check "String" and # todo
                    HttpValueNode Node;
                    Node.ModuleName = DefModuleList[i].name;
                    Node.object = DefModuleList[i].id[j];
                    Node.defNo = i;

                    int tempPos = (int) pos + 10;
                    int BeginPos, EndPos;
                    while (DefModuleList[i].value[j][tempPos] == ':' || DefModuleList[i].value[j][tempPos] == '.') {
                        tempPos++;
                    }
                    while (DefModuleList[i].value[j][tempPos] != '(') {
                        tempPos++;
                    }
                    //Node.method = DefModuleList[i].value[j].substr(BeginPos, EndPos - BeginPos + 1);
                    switch (f){
                        case 0:
                            Node.method = "Get";
                            break;
                        case 1:
                            Node.method = "Delete";
                            break;
                        case 2:
                            Node.method = "Put";
                            break;
                        default:
                            break;
                    }

                    tempPos++;
                    while (DefModuleList[i].value[j][tempPos] != ')') {
                        BeginPos = tempPos;
                        while (DefModuleList[i].value[j][tempPos] != ',' && DefModuleList[i].value[j][tempPos] != ')') {
                            tempPos++;
                        }
                        EndPos = tempPos - 1;
                        Node.value.push_back(DefModuleList[i].value[j].substr(BeginPos, EndPos - BeginPos + 1));
                        if (DefModuleList[i].value[j][tempPos] == ',') {
                            tempPos++;
                        }
                        while (DefModuleList[i].value[j][tempPos] == ' ') {
                            tempPos++;
                        }
                    }
                    HttpValueList.push_back(Node);
                }
            }
        }
    }
}

void GetModule(const string& RubyParse)
{
    int tempPos = 0;
    while(RubyParse[tempPos] == ' '){
        tempPos++;
    }
    if(RubyParse[tempPos] == '#'){
        return;
    } // #

    DefNode defNode;

    if(RubyParse.substr(tempPos, 3) == "def"){
        // Get def Module
        DefModule = 1;
        DefNo ++;
        // def fetch_details_from_external_service(isbn, id, headers)
        tempPos = tempPos + 3;
        while(RubyParse[tempPos] == ' '){
            tempPos++;
        }

        int BeginPos, EndPos;
        BeginPos = tempPos;
        while(RubyParse[tempPos] != '('){
            tempPos++;
        }
        EndPos = tempPos - 1;
        defNode.name = RubyParse.substr(BeginPos, EndPos-BeginPos+1);

        tempPos++;
        while(RubyParse[tempPos] != ')'){
            BeginPos = tempPos;
            while(RubyParse[tempPos] != ',' && RubyParse[tempPos] != ')'){
                tempPos++;
            }
            EndPos = tempPos - 1;
            defNode.paras.push_back(RubyParse.substr(BeginPos, EndPos-BeginPos+1));
            if(RubyParse[tempPos] == ','){
                tempPos++;
            }
            while(RubyParse[tempPos] == ' '){
                tempPos++;
            }
        }
        DefModuleList.push_back(defNode);
    }
}

void GetModuleValue(const string& RubyParse)
{
    int temp = 0;
    while(RubyParse[temp] == ' '){
        temp++;
    }
    if(RubyParse[temp] == '#'){
        return;
    } // #

    string::size_type Pos;
    Pos = RubyParse.find("begin");
    if(Pos != std::string::npos && RubyParse.length() == Pos+5 && RubyParse[Pos-1] == ' '){
        DefModule++;
    }
    Pos = RubyParse.find("then");
    if(Pos != std::string::npos && RubyParse.length() == Pos+4 && RubyParse[Pos-1] == ' '){
        DefModule++;
    }
    Pos = RubyParse.find("end");
    if(Pos != std::string::npos && RubyParse.length() == Pos+3 && (RubyParse[Pos-1] == ' ' || Pos == 0)){
        DefModule--;
    }

    int BeginPos, EndPos;
    Pos = RubyParse.find('=');
    if(Pos != std::string::npos && RubyParse[Pos+1] == ' ' && RubyParse[Pos-1] == ' '){
        int tempPos = 0;
        while(RubyParse[tempPos] == ' '){
            tempPos++;
        }
        BeginPos = tempPos;
        EndPos = (int)Pos-1;
        while(RubyParse[EndPos] == ' '){
            EndPos--;
        }
        DefModuleList[DefNo].id.push_back(RubyParse.substr(BeginPos, EndPos-BeginPos+1));
        tempPos = (int)Pos + 1;
        while(RubyParse[tempPos] == ' '){
            tempPos++;
        }
        DefModuleList[DefNo].value.push_back(RubyParse.substr(tempPos, RubyParse.length()-tempPos));
    }
    else if(Pos != std::string::npos && RubyParse[Pos+1] == '>' && RubyParse[Pos-1] == ' '){
        int tempPos = 0;
        while(RubyParse[tempPos] == ' '){
            tempPos++;
        }
        DefModuleList[DefNo].id.push_back(RubyParse.substr(tempPos, (int)Pos-tempPos));
        tempPos = (int)Pos + 2;
        while(RubyParse[tempPos] == ' '){
            tempPos++;
        }
        DefModuleList[DefNo].value.push_back(RubyParse.substr(tempPos, RubyParse.length()-tempPos));

    }
    else{
        if(RubyParse.length() == 0){
            return;
        }
        int tempPos = 0;
        while(RubyParse[tempPos] == ' '){
            tempPos++;
        }
        DefModuleList[DefNo].id.push_back(RubyParse.substr(tempPos, RubyParse.length()-tempPos));
        DefModuleList[DefNo].value.push_back(RubyParse.substr(tempPos, RubyParse.length()-tempPos));
    }

}

void GetValue()
{
    string flagstr = "URI.parse(";
    vector<stringNode> Str;
    for(int i=0; i<HttpValueList.size(); i++){
        for(int j=0; j<DefModuleList[HttpValueList[i].defNo].id.size(); j++){
            string::size_type pos = DefModuleList[HttpValueList[i].defNo].value[j].find(flagstr);
            if(pos != std::string::npos) {
                int tempPos = (int)pos + 10;
                int BeginPos = tempPos;
                string temp = DefModuleList[HttpValueList[i].defNo].value[j].substr(BeginPos, DefModuleList[HttpValueList[i].defNo].value[j].length()-BeginPos-1);
                Str = CheckAllString(temp);

                for(int z=0; z<Str.size(); z++){
                    if(Str[z].type == 1){
                        string value = findVarValue(Str[z].value, HttpValueList[i].defNo);
                        Str[z].value = value;
                        Str[z].type = 0;
                    }
                }

                string FinalStr;
                for(int z=0; z<Str.size(); z++){
                    FinalStr = FinalStr.append(Str[z].value);
                }
                HttpValueList[i].value.push_back(FinalStr);
            }
        }
    }
}

string findVarValue(const string& str, int defNo)
{
    string res;
    int CheckPara = 0;
    // check para
    for(int i=0; i<DefModuleList[defNo].paras.size(); i++){
        if(str == DefModuleList[defNo].paras[i]){
            CheckPara = 1;
        }
    }

    if(CheckPara == 1){
        res = findVarValue(str, DefModuleList[defNo].ParentMod[0]);
    }
    else{
        for(int j=0; j<DefModuleList[defNo].id.size(); j++){
            if(str.compare(DefModuleList[defNo].id[j]) == 0){
                res = DefModuleList[defNo].value[j].substr(1, DefModuleList[defNo].value[j].size()-2);
            }
        }
    }

    return res;
}

void GetRelation()
{
    ofstream file;
    file.open("relation.txt", ios::out);
    for(int i=0; i<=DefNo; i++){
        string str;
        str = DefModuleList[i].name;
        str.append(": ");
        file << str << endl;

        for(int j=0; j<=DefNo; j++){
            for(int k=0; k<DefModuleList[i].id.size(); k++){
                if(DefModuleList[i].value[k].find(DefModuleList[j].name) != std::string::npos){
                    file << "   " << DefModuleList[j].name << endl;
                    DefModuleList[j].ParentMod.push_back(i);
                }
            }
        }
        file << endl;
    }
    file.close();
}

void SetFlag()
{
    FlagStr.emplace_back("Net::HTTP::Get");
    FlagStr.emplace_back("Net::HTTP::Delete");
    FlagStr.emplace_back("Net::HTTP::Put");
}

void DebugHttpList()
{
    for(int i=0; i<HttpValueList.size(); i++){
        cout << i+1 << " : " << endl;
        cout << "   Module Name => " << HttpValueList[i].ModuleName << endl;
        cout << "   object => " << HttpValueList[i].object << endl;
        cout << "   method => " << HttpValueList[i].method << endl;
        for(const auto & j : HttpValueList[i].value){
            cout << "   value => " << j << endl;
        }
    }
}

void DebugDefModuleList()
{
    for(int i=0; i<DefModuleList.size(); i++){
        cout << i+1 << " : " << endl;
        cout << "   name => " << DefModuleList[i].name << endl;
        for(const auto & j : DefModuleList[i].paras){
            cout << "   paras => " << j << endl;
        }
        for(int j=0; j<DefModuleList[i].id.size(); j++){
            cout << "   id => " << DefModuleList[i].id[j] << "  value => " << DefModuleList[i].value[j] << endl;
        }
    }
}

void Print()
{
    //DebugHttpList();
    //DebugDefModuleList();
    for(int i=0; i<HttpValueList.size(); i++){
        cout << i+1 << " : " << endl;
        cout << "  object : " << HttpValueList[i].object << endl;
        cout << "  method : " << HttpValueList[i].method << endl;
        cout << "  url : " << HttpValueList[i].value[HttpValueList[i].value.size()-1] << endl;
    }
}
#include"hexplorer.h"
#include"hex_common.h"

class macro
{
private:
    char* nazwa;
    MSG* action[20000];
    int action_num;
    bool modified;
public:
    static int num;
    static macro* mac[100];
    static macro* recording;
    macro(char* n);
    ~macro();
    static void Activate(char* n);
    void AddAction(MSG* m);
    void Run();
    static void ReadAll();
    static void Clear();
};


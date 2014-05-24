#include <Windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include "FileSystem.cpp"
#include "MyClasses.cpp"
#include <CommCtrl.h>
#pragma comment(lib,"comctl32")
using namespace std;

class Application;
Application* app;
#define CAPTION 1
#define NAME 2
#define TEXT 0
class FileOpenBox
{
    HWND hWnd;
    int x,y,width,height;
public:
    FileOpenBox(int x,int y,int width,int height,HWND hParent)
    {
        this->x=x;
        this->y=y;
        this->width=width;
        this->height=height;
        
        this->hWnd=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_CHILD | WS_VISIBLE ,x,y,width,height,hParent,0,0,0);
        //SendMessage(hWnd,EM_SETLIMITTEXT,WPARAM(5),0);
    }
    string getLink()
    {
        char* t=new char[GetWindowTextLength(hWnd)+1];
        GetWindowTextA(hWnd,t,GetWindowTextLength(hWnd)+1);
        return string(t);
    }
};
class InfoBox
{
    HWND hWnd;
    HWND hParent;
    int x,y,width,height;
    string str;
    vector<HWND> savedItems;
public:
    InfoBox(int x,int y,int width,int height,HWND hParent)
    {
        this->x=x;
        this->y=y-20;
        this->width=width;
        this->height=height;
        this->hParent=hParent;
        CreateWindowEx(0,WC_LISTBOX,0,WS_CHILD | WS_VISIBLE,x,y,width,height,hParent,0,0,0);
        //this->hWnd=CreateWindowExA(WS_EX_CLIENTEDGE,"Edit",NULL,WS_CHILD | WS_VISIBLE | ES_MULTILINE ,x,y,width,height,hParent,0,0,0);
    }
    void addInfo(const string &info,int type=TEXT)
    {
        if(type==TEXT)
        {            
            str+=info;
            str+="\n";
            SetWindowTextA(hWnd,str.c_str());
        }
        else if(type==NAME)
        {
            y+=20;
            this->hWnd=CreateWindowExA(0,"Edit",NULL,WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,x+15,y,width-15,20,hParent,0,0,0);
            savedItems.push_back(this->hWnd);
            str=info;
            str+="\n";
            SetWindowTextA(hWnd,str.c_str());        
        }
        else
        {
            y+=20;
            this->hWnd=CreateWindowExA(0,"Edit",NULL,WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_CENTER,x+15,y,width-15,20,hParent,0,0,0);
            savedItems.push_back(this->hWnd);
            str=info;    
            SetWindowTextA(hWnd,str.c_str());     
        }
    }
    void clear()
    {
        str.clear();
        for(int i=0;i<savedItems.size();i++)
        {
            DestroyWindow(savedItems[i]);
            y-=20;
        }
        savedItems.clear();
        SetWindowTextA(hWnd,str.c_str());
    }
};
class FormatList
{
    HWND hWnd,hEdit,hButton,hDel;
    int x,y,width,height;
    vector<string> formats;
public:
    FormatList(int x,int y,int width,int height,HWND hParent)
    {
        this->x=x;
        this->y=y;
        this->width=width;
        this->height=height;
        CreateWindowExA(0,"STATIC",0,WS_CHILD | WS_VISIBLE,x,y,width,height,hParent,0,0,0);
        CreateWindowExA(0,"STATIC","Formats list",WS_CHILD | WS_VISIBLE,x+95,y+10,width,30,hParent,0,0,0);
        hWnd=CreateWindowExA(WS_EX_CLIENTEDGE,"LISTBOX",0,WS_CHILD | WS_VISIBLE | WS_VSCROLL | SS_CENTER,x,y+30,width,height-90,hParent,0,0,0);
        hEdit=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",0,WS_CHILD | WS_VISIBLE,x,y+height-60,width,30,hParent,0,0,0);
        hButton=CreateWindowExA(0,"BUTTON",0,WS_CHILD | WS_VISIBLE,x,y+height-30,width/2,30,hParent,0,0,0);
        hDel=CreateWindowExA(0,"BUTTON",0,WS_CHILD | WS_VISIBLE,x+width/2,y+height-30,width/2,30,hParent,0,0,0);
        SetWindowTextA(hButton,"Add");
        SetWindowTextA(hDel,"Del");
    }
    void push(const char* value)
    {
        string str=value;
        for(int i=0;i<formats.size();i++)
        {
            if(formats[i]==str) return;
        }
        formats.push_back(str);
        SetWindowTextA(hEdit,"");
        SendMessageA(this->hWnd,LB_ADDSTRING,0,LPARAM(value));
    }
    void buttonProc(WPARAM wp,LPARAM lp)
    {
        if(HWND(lp)==hButton)
        {
            char* buf=new  char[GetWindowTextLengthA(hEdit)+1];
            GetWindowTextA(hEdit,buf,GetWindowTextLengthA(hEdit)+1);
            this->push(buf);
        }
        else if(HWND(lp)==hDel)
        {
            delProc(wp,lp);
        }
    }
    void delProc(WPARAM wp,LPARAM lp)
    {       
        int id=(int)SendMessage(hWnd,LB_GETCURSEL,0,0);
        if(id!=LB_ERR)
        {
             formats.erase(formats.begin()+id);
            SendMessageA(hWnd,LB_DELETESTRING,WPARAM(id),0);
        }
    }
    friend class Application;
};
class Application
{ 
    HINSTANCE hInst;
    HWND hWnd;
    HWND hAddres,hButton;
    FileOpenBox* fileGetter;
    InfoBox* infoBox;
    FormatList* formatList;
#define WIDTH 800
#define HEIGHT 600
public:
    Application(HINSTANCE hInst)
    {
        WNDCLASSEX wcl;
        TCHAR szClassWindow[] = L"Project Manager";
        // 1. Определение класса окна
        wcl.cbSize = sizeof(wcl);	// размер структуры WNDCLASSEX
        // Перерисовать всё окно, если изменён размер по горизонтали или по вертикали
        wcl.style = CS_HREDRAW | CS_VREDRAW;	// CS (Class Style) - стиль класса окна
        wcl.lpfnWndProc = WindowProc;	// адрес оконной процедуры
        wcl.cbClsExtra = 0;		// используется Windows 
        wcl.cbWndExtra  = 0; 	// используется Windows 
        wcl.hInstance = hInst;	// дескриптор данного приложения
        wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// загрузка стандартной иконки
        wcl.hCursor = LoadCursor(NULL, IDC_ARROW);	// загрузка стандартного курсора	
        wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);	// заполнение окна белым цветом
        wcl.lpszMenuName = NULL;	// приложение не содержит меню
        wcl.lpszClassName = szClassWindow;	// имя класса окна
        wcl.hIconSm = NULL;	// отсутствие маленькой иконки для связи с классом окна

        // 2. Регистрация класса окна
        if (!RegisterClassEx(&wcl))
            return; // при неудачной регистрации - выход
        
        // 3. Создание окна
        // создается окно и  переменной hWnd присваивается дескриптор окна
        hWnd = CreateWindowEx(
            0,		// расширенный стиль окна
            szClassWindow,	//имя класса окна
            szClassWindow, // заголовок окна
            WS_OVERLAPPEDWINDOW,				// стиль окна
            /* Заголовок, рамка, позволяющая менять размеры, системное меню, кнопки развёртывания и свёртывания окна  */
            CW_USEDEFAULT,	// х-координата левого верхнего угла окна
            CW_USEDEFAULT,	// y-координата левого верхнего угла окна
            WIDTH+15,	// ширина окна
            HEIGHT+38,	// высота окна
            NULL,			// дескриптор родительского окна
            NULL,			// дескриптор меню окна
            hInst,		// идентификатор приложения, создавшего окно
            NULL);		// указатель на область данных приложения
        // 4. Отображение окна
        SetTimer(hWnd,0,10,0);
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd); // перерисовка окна
        
        PostMessage(hWnd,WM_INITDIALOG,0,0);

    }
    void work()
    {
        MSG lpMsg;
        while(GetMessage(&lpMsg, NULL, 0, 0)) // получение очередного сообщения из очереди сообщений
        {
            TranslateMessage(&lpMsg);	// трансляция сообщения
            DispatchMessage(&lpMsg);	// диспетчеризация сообщений
        }
    }
    static void init()
    {
        app->fileGetter=new FileOpenBox(0,0,2*WIDTH/3,30,app->hWnd);
        app->infoBox=new InfoBox(0,60,2*WIDTH/3,HEIGHT-60,app->hWnd);
        app->formatList=new FormatList(2*WIDTH/3+1,0,WIDTH/3,HEIGHT,app->hWnd);
        app->hButton=CreateWindowExA(WS_EX_CLIENTEDGE,"BUTTON",NULL,WS_CHILD | WS_VISIBLE,0,30,2*WIDTH/3,30,app->hWnd,0,0,0);
        SetWindowTextA(app->hButton,"Count");   
        app->formatList->push("cpp");app->formatList->push("c");app->formatList->push("js");app->formatList->push("as");app->formatList->push("php");
        KillTimer(app->hWnd,0);
    } 
    void getSizes(vector<MyFile>* files,__int64 &min,__int64 &max,__int64 &total,__int64 &average)
    {
        total=min=max=files->at(0).fileSize();
        for(int i=1;i<files->size();i++)
        {
            total+=files->at(i).fileSize();
            if(files->at(i).fileSize()<min) min=files->at(i).fileSize();
            else if(files->at(i).fileSize()>max) max=files->at(i).fileSize();
            
        }
        average=total/files->size();
    }
    int getNumLines(vector<MyFile>* files,int &min,int &max,int &average,int &total)
    {
        ifstream* file=new ifstream();
        int rez=0;
        char* t=new char[5000];
        total=0;
        file->open(files->at(0).fullPath);
        while(!file->fail())
        {
            file->getline(t,5000);
            rez++;
        }
        total=min=max=rez;
        for(int i=1;i<files->size();i++)
        {
            rez=0;
            file=new ifstream();
            file->open(files->at(i).fullPath);
            while(!file->fail())
            {
                file->getline(t,5000);
                rez++;
            }
            if(rez>max) max=rez;
            if(rez<min) min=rez; 
            total+=rez;
            file->close();
            delete file;
        }
        average=total/files->size();
        delete[] t;
        return rez;
    }
    std::vector<MyFile>* filter(std::vector<MyFile>* filesFull,string &format)
    {
        auto rez=new std::vector<MyFile>();
        for(int i=0;i<filesFull->size();i++)
        {
            if(filesFull->at(i).fullPath.substr(filesFull->at(i).fullPath.find_last_of('.')+1)==format)
            {
                rez->push_back(filesFull->at(i));
            }
        }
        return rez;
    }
    void count(WPARAM wp,LPARAM lp)
    {
        if(HWND(lp)==hButton)
        {
            app->infoBox->clear();
            string project=app->fileGetter->getLink();
            if(project.length()==0)
            {
                MessageBoxA(0,"Incorect path","Error",MB_OK | MB_ICONERROR);
                return;                
            }
            string currentSearch;
            char* bufferStr=new char[20];
            vector<MyFile>* files;
            __int64 min,max,average,total;
            int minL,maxL,averageL,totalL;
            __int64 numFilesP=0,totalP=0,totalLP=0;
            int numLines;
            currentSearch=project+"\\*.*";
            auto filesFull=FileSystem::findDirectoryContentEx(currentSearch);
            if(filesFull->size()==0)
            {
                MessageBoxA(0,"Incorect path","Error",MB_OK | MB_ICONERROR);
                return;
            }
            if(formatList->formats.size()==0)
            {
                MessageBoxA(0,"No file formats selected","Error",MB_OK | MB_ICONERROR);
                return;
            }
            for(int i=0;i<app->formatList->formats.size();i++)
            {
                auto files=filter(filesFull,formatList->formats[i]);
                if(files->size()==0) continue;
                infoBox->addInfo(app->formatList->formats[i]+" files statistics",CAPTION);
                itoa(files->size(),bufferStr,10);
                infoBox->addInfo("Num files: ",NAME);

                infoBox->addInfo(bufferStr,TEXT);
                infoBox->addInfo("Average size: ",NAME);

                getSizes(files,min,max,total,average);
                itoa(average,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Max size: ",NAME);
                itoa(max,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Min size: ",NAME);
                itoa(min,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Total size: ",NAME);
                itoa(total,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                numLines=getNumLines(files,minL,maxL,averageL,totalL);
                infoBox->addInfo("Minimum lines: ",NAME);
                itoa(minL,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Maximum lines: ",NAME);
                itoa(maxL,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Average lines: ",NAME);
                itoa(averageL,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Total lines: ",NAME);
                itoa(totalL,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                infoBox->addInfo("Chars per line: ",NAME);
                itoa(total/totalL,bufferStr,10);
                infoBox->addInfo(bufferStr,TEXT);

                totalP+=total;
                totalLP+=totalL;
                numFilesP+=files->size();
                files->clear();
            }
            infoBox->addInfo("Project statistics",CAPTION);
            infoBox->addInfo("Total size: ",NAME);
            itoa(totalP,bufferStr,10);
            infoBox->addInfo(bufferStr,TEXT);

            infoBox->addInfo("Average file size: ",NAME);
            itoa(totalP/numFilesP,bufferStr,10);
            infoBox->addInfo(bufferStr,TEXT);

            infoBox->addInfo("Num files: ",NAME);
            itoa(numFilesP,bufferStr,10);
            infoBox->addInfo(bufferStr,TEXT);

            infoBox->addInfo("Total lines: ",NAME);
            itoa(totalLP,bufferStr,10);
            infoBox->addInfo(bufferStr,TEXT);

            infoBox->addInfo("Lines per file: ",NAME);
            itoa(totalLP/numFilesP,bufferStr,10);
            infoBox->addInfo(bufferStr,TEXT);


            delete[] bufferStr;
        }
    }
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
    {
        int result;
        switch(uMessage)
        {
        case WM_INITDIALOG:
            break;
        case WM_CHAR:
            { 
                 break;
            }
        case WM_COMMAND:
            {
                app->formatList->buttonProc(wParam,lParam);
                app->count(wParam,lParam);
                break;
            }
        case WM_TIMER:
            init();
            break;
        case WM_DESTROY: // сообщение о завершении программы
            PostQuitMessage(0); // посылка сообщения WM_QUIT
            break;
        default:
            // все сообщения, которые не обрабатываются в данной оконной функции 
            // направляются обратно Windows на обработку по умолчанию
            return DefWindowProc(hWnd, uMessage, wParam, lParam);
        }
        return 0;
    }
};

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
    app=new Application(hInst);
    app->work();
}
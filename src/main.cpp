//#define FPS
//#define BigScreen
#include "FeEGELib.h"
using namespace FeEGE;

int main(){
//	setcaption("Vocabulary Selector（触屏修复版）");
	setcaption("Vocabulary Selector");
	HDC hdc = GetDC(NULL);
	int width = GetDeviceCaps(hdc, HORZRES);
    int height = GetDeviceCaps(hdc, VERTRES);
	init(width,height,INIT_NOBORDER | INIT_RENDERMANUAL);
	initpen();
	setbkcolor(EGERGB(50,50,50));
	pen::font(70,"幼圆");
	pen::type(PenType.middle);
	
	Element* exitButton = newElement("exit",".\\resources\\image\\exit.png",80,Y - 80);
	Element* nextButton = newElement("next",".\\resources\\image\\next.png",X - 80,Y - 80);
	Element* prevButton = newElement("prev",".\\resources\\image\\prev.png",X - 80 - 512 * 0.2,Y - 80);
	Element* settings = newElement("settings",".\\resources\\image\\setting.png",X * 0.5 - 512 * 0.2 *1.5,Y - 80);
	Element* RebuildNameChooser = newElement("RebuildNameChooser",".\\resources\\image\\RebuildNameChooser.png",X * 0.5 + 512 * 0.2 *1.5,Y - 80);
	
	long long Total = 0,Pos = 0;
	std::vector<std::string> words;
	mt19937 rng(time(0));
	
	auto reloadDict = [&](){
		std::string word;
		Pos = 0;
		Total = 0;
		words.clear();
		ifstream in;
		in.open(".\\config\\dict.txt",ios::in);
		while(getline(in,word)){
			words.push_back(word);
			Total ++;
		}
		if(Total){
			Pos = 1;
			shuffle(words.begin(),words.end(),rng);
		}
	};
	
	auto reDraw = [&](){
		if(Pos == 0 || Pos == 1) prevButton->hide();
		else prevButton->show();
		if(Pos == Total) nextButton->hide();
		else nextButton->show();
		
		pen::clear_all();
		pen::font(70,"幼圆");
		pen::color(RED);
		pen::print(X >> 1,Y * 0.15,"No. " + to_string(Pos) + " / " + to_string(Total));
		pen::font(100,"幼圆");
		pen::color(WHITE);
		pen::print(X >> 1,Y >> 1,words[Pos - 1]);
	};
	
	auto runcmd = [&](const std::string cmd){
		STARTUPINFO si = { sizeof(si) };
	    PROCESS_INFORMATION pi;
	
	    si.dwFlags = STARTF_USESHOWWINDOW;
	    si.wShowWindow = SW_SHOWDEFAULT;
	    const char* command = cmd.c_str();
        CreateProcess(NULL, (LPSTR)command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
	};
	
	exitButton->show();
	exitButton->set_scale(20);
	exitButton->listen(EventType.on_click,"exit",[&](Element* e){
		closeGraph = true;
	});
	
	nextButton->show();
	nextButton->set_scale(20);
	nextButton->listen(EventType.on_click,"next",[&](Element* e){
		Pos ++;
		reDraw();
	});
	
	prevButton->show();
	prevButton->set_scale(20);
	prevButton->listen(EventType.on_click,"prev",[&](Element* e){
		Pos --;
		reDraw();
	});
	
	settings->show();
	settings->set_scale(20);
	settings->listen(EventType.on_click,"settings",[&](Element* e){
        int result = MessageBox(
	        getHWnd(),
			TEXT("即将打开配置文件，关闭配置文件后自动重载，是否继续"),
	        TEXT("提示"),
	        MB_YESNO | MB_ICONQUESTION
	    );
	    if(result == IDYES){
	    	runcmd("notepad.exe .\\config\\dict.txt");
	        reloadDict();
	        reDraw();
		}
	});
	
//	settings->listen(EventType.frame,"cout",[&](Element* e){
//		cout<<e->get_variable(0)<<"\n";
//	});
	
	RebuildNameChooser->show();
	RebuildNameChooser->set_scale(488);
	RebuildNameChooser->listen(EventType.on_click,"click",[&](Element* e){
		runcmd("RebuildNameChooser.exe");
	});
	
	reloadDict();
	reDraw();
	start();
	return 0;
}

//#define FPS
//#define BigScreen
#include "FeEGELib.h"
#include <tlhelp32.h>
using namespace FeEGE;

long long Total = 0,Pos = 0,Volume = 100,Speed = 50;
std::vector<std::string> words;
mt19937 rng(time(0));

std::string cleanText(const std::string& input) {
    std::string output;
    for (char ch : input) {
        if (std::isalpha(static_cast<unsigned char>(ch)) || std::isspace(static_cast<unsigned char>(ch))) {
            output += ch;
        }
        // 可选：保留数字
        else if (std::isdigit(static_cast<unsigned char>(ch))) {
            output += ch;
        }
        else if(ch == '.' || ch == ',' || ch == '?' || ch == '!'){
        	output += ch;
		}
        // 否则跳过符号
    }
    return output;
}

void speak(const std::string& text, const std::string& voice = "Microsoft Zira Desktop") {
    std::string clean = cleanText(text);
    std::string voiceCopy = voice;

    std::thread([clean, voiceCopy]() {
    	int sapiSpeed = static_cast<int>(Speed / 100.0 * 20.0 - 10);  // 映射到 [-10, 10]
        std::string cmdArgs = "-n \"" + voiceCopy + "\" -t \"" + clean + "\""
                    + " -v " + std::to_string(Volume)
                    + " -s " + std::to_string(sapiSpeed);

        ShellExecuteA(
            NULL,
            "open",
            resolvePath(".\\balcon.exe").c_str(),
            cmdArgs.c_str(),
            NULL,
            SW_HIDE
        );
    }).detach();
}

void killBalcon() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { sizeof(pe) };

    if (Process32First(hSnap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, "balcon.exe") == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 1);
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
}

int main(int argc,char** argv){
	string path = getCurrentPath();
	if(argc == 2) path = string(argv[1]);
	else path = resolvePath(".\\config\\dict.txt");
#ifdef BigScreen
	setcaption("Vocabulary Selector（触屏修复版）");
#else
	setcaption("Vocabulary Selector");
#endif
	HDC hdc = GetDC(NULL);
	int width = GetDeviceCaps(hdc, HORZRES);
    int height = GetDeviceCaps(hdc, VERTRES);
	init(width,height,INIT_NOBORDER | INIT_RENDERMANUAL);
	initPen();
	setbkcolor(EGERGB(50,50,50));
	pen::font(70,"宋体");
	pen::type(PenType.middle);
	
	auto reloadDict = [&](){
		std::string word;
		Pos = 0;
		Total = 0;
		words.clear();
		ifstream in;
		in.open(path,ios::in);
		bool flag = true,NoRandom = false;
		while(getline(in,word)){
			if(detectEncoding(word) == "UTF-8"){
				word = UTF8ToANSI(word);
			}
			int A = 0,B = word.length();
			for(;A < word.length();++ A){
				if(word[A] != ' ') break;
			}
			for(;B >= A;-- B){
				if(word[B - 1] != ' ') break;
			}
			if(A == B || word.length() == 0) continue;
			word = word.substr(A,B);
			if(flag && (word == "#comment NoRandom" || word == "NoRandom")){
				NoRandom = true;
				flag = false;
				continue;
			}
			if(flag) flag = false;
			words.push_back(word);
			Total ++;
		}
		if(Total){
			Pos = 1;
			if(!NoRandom) shuffle(words.begin(),words.end(),rng);
		}
	};
	
	auto reDraw = [&](){
		Button* prevButton = dynamic_cast<Button*>(getWidgetById("prevButton"));
		Button* nextButton = dynamic_cast<Button*>(getWidgetById("nextButton"));
		Button* voiceButton = dynamic_cast<Button*>(getWidgetById("voiceButton"));
		if(Pos == 0 || Pos == 1) prevButton->setScale(0);
		else prevButton->setScale(1);
		if(Pos == Total) nextButton->setScale(0);
		else nextButton->setScale(1);
		if(Pos == 0) voiceButton->setScale(0);
		else voiceButton->setScale(1);
		pen::clearAll();
		pen::font(70,"宋体");
		pen::color(EGERGB(0,255,255));
		pen::print(X >> 1,Y * 0.15,"No. " + to_string(Pos) + " / " + to_string(Total));
		if(Pos == 0) return; 
		int F = 101;
		do{
			F --;
			pen::font(F,"宋体");
		} while(textwidth(words[Pos - 1].c_str(),pen_image) >= X);
		pen::color(WHITE);
		pen::print(X >> 1,Y >> 1,words[Pos - 1]);
	};	
	
	ButtonBuilder()
		.setIdentifier("exitButton")
		.setCenter(80,Y - 80)
		.setColor(EGERGBA(119,136,153,255))
		.setIcon(getImage(resolvePath(".\\resources\\image\\exit.png")))
		.setSize(100,100)
		.setIconSize(20)
		.setRadius(12)
		.setContent("")
		.setOnClick([&](){
			killBalcon();
			closeGraph = true;
		})
		.build();

	ButtonBuilder()
		.setIdentifier("nextButton")
		.setCenter(X - 80,Y - 80)
		.setColor(EGERGBA(119,136,153,255))
		.setIcon(getImage(resolvePath(".\\resources\\image\\next.png")))
		.setSize(100,100)
		.setIconSize(20)
		.setRadius(12)
		.setContent("")
		.setOnClick([&](){
			Pos ++;
			reDraw();
		})
		.build();

	ButtonBuilder()
		.setIdentifier("voiceButton")
		.setCenter(X - 80,Y - 190)
		.setColor(EGERGBA(119,136,153,255))
		.setIcon(getImage(resolvePath(".\\resources\\image\\voice.png")))
		.setSize(100,100)
		.setIconSize(20)
		.setRadius(12)
		.setContent("")
		.setOnClick([&](){
			if(Pos >= 1){
				killBalcon();
				speak(words[Pos - 1]);
			} 
		})
		.build();
	
	ButtonBuilder()
		.setIdentifier("prevButton")
		.setCenter(X - 80 - 512 * 0.2,Y - 80)
		.setColor(EGERGBA(119,136,153,255))
		.setIcon(getImage(resolvePath(".\\resources\\image\\prev.png")))
		.setSize(100,100)
		.setIconSize(20)
		.setRadius(12)
		.setContent("")
		.setOnClick([&](){
			Pos --;
			reDraw();
		})
		.build();

	ButtonBuilder()
		.setIdentifier("settings")
		.setCenter(X * 0.5 - 512 * 0.2 * 0.5,Y - 80)
		.setColor(EGERGBA(119,136,153,255))
		.setIcon(getImage(resolvePath(".\\resources\\image\\setting.png")))
		.setSize(100,100)
		.setIconSize(20)
		.setRadius(12)
		.setContent("")
		.setOnClick([&](){
			int result = MessageBox(
				getHWnd(),
				TEXT("即将打开配置文件，关闭配置文件后自动重载，是否继续"),
				TEXT("提示"),
				MB_YESNO | MB_ICONQUESTION
			);
			if(result == IDYES){
				runCmdAwait("notepad.exe " + path);
				reloadDict();
				reDraw();
			}
		})
		.build();

	ButtonBuilder()
		.setIdentifier("RebuildNameChooser")
		.setCenter(X * 0.5 + 512 * 0.2 * 0.5,Y - 80)
		.setColor(EGERGBA(119,136,153,255))
		.setIcon(getImage(resolvePath(".\\resources\\image\\RebuildNameChooser.png")))
		.setSize(100,100)
		.setIconSize(200)
		.setRadius(12)
		.setContent("")
		.setOnClick([&](){
#ifdef BigScreen
			ShellExecute(NULL, "open", resolvePath(".\\RebuildNameChooser HD.exe").c_str(), NULL, NULL, SW_SHOWDEFAULT);
#else 
			ShellExecute(NULL, "open", resolvePath(".\\RebuildNameChooser.exe").c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
			cout<<resolvePath(".\\RebuildNameChooser HD.exe")<<"\n";
		})
		.build();

	SliderBuilder()
		.setCenter(X - 80,Y - 365)
		.setOrientation(Orientation::Column)
		.setSize(35,160)
		.setThickness(2.5)
		.setOnChange([&](double t){
			Volume = 100 * t;
			Text* txt = dynamic_cast<Text*>(getWidgetById("volumeString"));
			int r = static_cast<int>(Volume * 255.00f / 100);         // 越大越红
			int g = static_cast<int>((100 - Volume) * 255.00f / 100); // 越小越绿
			int b = 0;

			txt->setContent("音量 ：" + std::to_string(Volume) + "%");
			txt->setColor(EGERGB(r, g, b));
		})
		.setProgress(1.00f)
		.build();
	
	TextBuilder()
		.setIdentifier("volumeString")
		.setContent("音量 ：" + std::to_string(Volume) + "%")
		.setColor(RED)
		.setAlign(TextAlign::Center)
		.setPosition(X - 80,Y - 270)
		.setFont(22,"宋体")
		.build();
		
	SliderBuilder()
		.setCenter(X - 220,Y - 365)
		.setOrientation(Orientation::Column)
		.setSize(35,160)
		.setThickness(2.5)
		.setOnChange([&](double t){
			Speed = 100 * t;
			Text* txt = dynamic_cast<Text*>(getWidgetById("speedString"));
			int r = static_cast<int>(Speed * 255.00f / 100);         // 越大越红
			int g = static_cast<int>((100 - Speed) * 255.00f / 100); // 越小越绿
			int b = 0;

			txt->setContent("语速 ：" + std::to_string(Speed) + "%");
			txt->setColor(EGERGB(r, g, b));
		})
		.setProgress(0.50f)
		.build();
	
	TextBuilder()
		.setIdentifier("speedString")
		.setContent("语速 ：" + std::to_string(Speed) + "%")
		.setColor(RED)
		.setAlign(TextAlign::Center)
		.setPosition(X - 220,Y - 270)
		.setFont(22,"宋体")
		.build();

	reloadDict();
	reDraw();
	start();
	return 0;
}

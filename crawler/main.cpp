#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include<string>
#include<regex>
#include<vector>


std::string Global_hostname = "store.line.me";

#include <windows.h>
#include <string>

std::wstring utf8_to_wstring(const std::string& s) {    //gpt幫我生成的 嘿嘿
    int len = MultiByteToWideChar(
        CP_UTF8, 0,
        s.c_str(), -1,
        nullptr, 0
    );

    std::wstring ws(len, 0);
    MultiByteToWideChar(
        CP_UTF8, 0,
        s.c_str(), -1,
        &ws[0], len
    );

    ws.pop_back(); // 去掉結尾的 \0
    return ws;
}


int Exportdata(std::string inbuffer){ //本來我是想用網址當作檔名就好，但是system不允許奇怪字元，只好進來再做處理
    std::string titlename;  //檔案名稱，就是貼圖標題
    std::string filecontent;   //檔案內容
    std::regex pattern(R"(<p class="[^\s]+" data-test="sticker-name-title">([^<]+)</p>)");
    std::sregex_iterator begin(inbuffer.begin(),inbuffer.end(),pattern);
    std::sregex_iterator end;
    for(std::sregex_iterator it = begin;it != end;it++){
        //printf("(debug)[Exportdata](*it)[1].str() = %s\n",(*it)[1].str());
        titlename = (*it)[1].str();
        //printf("(debug)[Exportdata]pathname: ");
        //std::cout << titlename << std::endl;
    }
    //到這邊為止已經可以取得檔案名稱
    //接下來要將抓到的內容分析並製作成表格型的html

    //抓出所有貼圖的url放進vec_url
    std::vector<std::string> vec_url;
    pattern = std::regex(R"(<div class="[^\s]+ FnImage">[\s]+<span class="mdCMN09Image" style="background-image:url\(([^\s]+)\?.+?</span>)");
    begin = std::sregex_iterator(inbuffer.begin(),inbuffer.end(),pattern);

    for(auto it = begin;it != end;it++){
        //printf("(debug)[Exportdata]find:");
        //std::cout << (*it)[1].str() << std::endl;
        vec_url.push_back((*it)[1].str());
    }

    //for(int i = 0;i < vec_url.size();i++) std::cout << vec_url[i] << std::endl;


    //抓取範例html
    FILE* fp = fopen("template.html","r");
    if(fp){
        int filesize = 0;
        fseek(fp,0,SEEK_END);
        filesize = ftell(fp);
        fseek(fp,0,SEEK_SET);
        char* buffer;
        std::string strbuffer;
        buffer = new char[filesize];
        fread(buffer,1,filesize,fp);
        //printf("(debug)[Exportdata]buffer:\n");
        //for(int i = 0;i < filesize;i++) printf("%c",buffer[i]);
        strbuffer = buffer;

        //找前段
        pattern = std::regex(R"(([\s\S]+?<h1>)@stickername(</h1>[\s]+<table>[\s]+))");
        begin = std::sregex_iterator(strbuffer.begin(),strbuffer.end(),pattern);
        for(auto it = begin;it != end;it++){
            filecontent = (*it)[1].str() + titlename + (*it)[2].str();
        }

        //塞中間
        for(int i = 0;i < vec_url.size();i++){
            std::string btnid = "btn-" + std::to_string(i);
            std::string urlid = "url-" + std::to_string(i);
            filecontent += R"(<tr class="@replacehere">
                <td>
                    <img src=")"+ vec_url[i] + R"("/>
                </td>
                <td id=")"+ urlid  + R"(">
                    )" + vec_url[i] + R"( 
                </td>
                <td>
                    <button id=")"+ btnid + R"(">copy</button>
                </td>
            </tr>)";
        }
        //找後段
        pattern = std::regex(R"([\s\S]+(<\/table>[\s\S]+?<\/html>))");
        begin = std::sregex_iterator(strbuffer.begin(),strbuffer.end(),pattern);
        for(auto it = begin;it != end;it++){
            filecontent += (*it)[1].str();
        }

        delete[] buffer;
    }
    else{
        printf("(error)[Exportdata]fp is null\n");
    }
    fclose(fp);
    

    //filecontent = inbuffer;
    //最終寫入
    fp = _wfopen(utf8_to_wstring((titlename+".html").c_str()).c_str(),L"w");
    if(fp){
        fwrite(filecontent.c_str(),1,filecontent.size(),fp);
        fclose(fp);
    }
    else{
        printf("(error)[Exportdata]fp is null\n");
    }
    
    return 0;
}

int Crawler(std::wstring pathname){
    std::wstring hostname(Global_hostname.begin(),Global_hostname.end());
    char* pbuffer = nullptr; //放整個檔的，因為下面的buffer是分段抓
    std::string strbuffer;



HINTERNET hSession = WinHttpOpen(
        L"MyCrawler/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if(!hSession)
        std::cerr << "WinHttpOpen failed, error: " << GetLastError() << "\n"; 

    HINTERNET hConnect = WinHttpConnect(
        hSession,
        hostname.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );

    if(!hConnect)
        std::cerr << "WinHttpConnect failed, error: " << GetLastError() << "\n"; 

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        pathname.c_str(),
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE   // 🔥 關鍵：HTTPS
    );
    std::wcout << pathname << std::endl;
    if(!hRequest)
        std::cerr << "WinHttpOpenRequest failed, error: " << GetLastError() << "\n"; 

    if(!WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    ))
        std::cerr << "WinHttpSendRequest failed, error: " << GetLastError() << "\n"; 

    if(!WinHttpReceiveResponse(hRequest, NULL))
        std::cerr << "Receive response failed, error: " << GetLastError() << "\n"; 

    //---
    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    if (WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &size,
        WINHTTP_NO_HEADER_INDEX))
    {
        std::wcout << L"HTTP Status Code: " << statusCode << std::endl;
    }
    else {
        std::cerr << "Query status code failed\n";
    }
    //----

    DWORD dwSize = 0;

    do {

        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (!dwSize) break;

        char* buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);

        DWORD dwRead = 0;
        WinHttpReadData(hRequest, buffer, dwSize, &dwRead);
        //std::cout << buffer;
        strbuffer += buffer;

        delete[] buffer;
    } while (dwSize > 0);

    Exportdata(strbuffer);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return 0;
}

int Getrunlist(std::vector<std::wstring> &vec_runlist){
    int filesize = 0;
    char* buffer;
    std::string sbuffer;
    FILE* fp =  fopen("runlist.txt","r");
    fseek(fp,0,SEEK_END);
    filesize = ftell(fp);
    buffer = new char[filesize];
    //printf("(debug)[Getrunlist]filesize = %d\n",filesize);
    fseek(fp,0,SEEK_SET);
    fread(buffer,1,filesize,fp);
    fclose(fp);
    sbuffer = buffer;
    //std::cout << sbuffer << endl;


    std::regex pattern(R"(https?://[^/]+?(/[^\s]+))");  //因為winhttp的用法，hostname可以先固定就好，故這邊只抓pathname
    std::sregex_iterator begin(sbuffer.begin(),sbuffer.end(),pattern);
    std::sregex_iterator end;

    for(std::sregex_iterator it = begin;it != end;it++){
        std::string tempstr = (*it)[1].str();
        std::wstring ws(tempstr.begin(),tempstr.end());
        vec_runlist.push_back(ws);
    }

    return 0;
}

int main() {
    std::vector<std::wstring> vec_runlist;

    Getrunlist(vec_runlist);
    //printf("(debug)[main]vec_runlist start\n");
    //for(int i = 0;i < vec_runlist.size();i++) std::wcout << vec_runlist[i] << std::endl;
    //printf("(debug)[main]vec_runlist end\n");
    for(auto it = vec_runlist.begin();it != vec_runlist.end();it++) 
        Crawler(*it); 
    return 0;
}

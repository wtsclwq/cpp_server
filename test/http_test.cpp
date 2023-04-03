//
// Created by wtsclwq on 23-4-3.
//

#include "../src/include/http/http_request.h"
#include "../src/include/http/http_response.h"

void test() {
    wtsclwq::HttpRequest::ptr req = std::make_shared<wtsclwq::HttpRequest>();
    req->SetHeader("host", "www.baidu.com/abc/aaa");
    req->SetPath("xxxxx");
    req->SetBody("hello wtsclwq");
    req->Dump(std::cout) << std::endl;
}

int main() {
    test();
    return 0;
}
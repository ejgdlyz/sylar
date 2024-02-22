#include "sylar/http/ws_connection.h"
#include "sylar/iomanager.h"
#include "sylar/util.h"

void run() {
    auto rt = sylar::http::WSConnection::Create("http://127.0.0.1:8020/sylar", 1000);
    if (!rt.second) {
        std::cout << rt.first->toString() << std::endl;
        return;
    }

    auto conn = rt.second;
    while (true) {
        for (int i = 0; i < 1; ++i) {
            // conn->sendMessage(sylar::random_string(60), sylar::http::WSFrameHead::TEXT_FRAME, false);
            conn->sendMessage("Hello WebSocket~~~~", sylar::http::WSFrameHead::TEXT_FRAME, false);
        }
        // conn->sendMessage(sylar::random_string(65), sylar::http::WSFrameHead::TEXT_FRAME, true);
        conn->sendMessage("WebSocket Finished", sylar::http::WSFrameHead::TEXT_FRAME, true);
        auto msg = conn->recvMessage();
        if (!msg) {
            break;
        }
        std::cout << "opcode=" << msg->getOpcode()
                << " data=" << msg->getData() << std::endl;
        
        sleep(10);
    }
}

int main(int argc, char const *argv[]) {
    srand(time(0));
    sylar::IOManager iom(1);
    iom.schedule(run);
    return 0;
}

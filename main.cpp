#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/StreamCopier.h"

#include <iostream>

#include "opencv2/opencv.hpp"
#include "baidu_face_api.h"


using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

BaiduFaceApi *api;

class HelloRequestHandler : public HTTPRequestHandler {
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
        Application &app = Application::instance();
        app.logger().information("Request from %s", request.clientAddress().toString());

        std::istream &in = request.stream();

        Poco::StreamCopier copier;
        std::string image;
        copier.copyToString(in, image);

        std::cerr << "http request " << image << std::endl;

        std::cerr << "image size " << image.length() << std::endl;


        const char *result = api->face_quality(image.c_str(), 1);

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");


        std::ostream &out = response.send();
        out << result;
        out.flush();
    }
};

class HelloRequestHandlerFactory : public HTTPRequestHandlerFactory {
    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &) {
        return new HelloRequestHandler;
    }
};

class WebServerApp : public ServerApplication {
    void initialize(Application &self) {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    int main(const std::vector<std::string> &) {
        //  UInt16 port = static_cast<UInt16>(config().getUInt("port", port));
        const std::vector<std::string>& argv = this->argv();
        std::string p = argv[1];

        UInt16 port = std::atoi(p.c_str());

        HTTPServer srv(new HelloRequestHandlerFactory, port);
        srv.start();
        logger().information("HTTP Server started on port %hu.", port);
        waitForTerminationRequest();
        logger().information("Stopping HTTP Server...");


        srv.stop();

        return Application::EXIT_OK;
    }
};


int main(int argc, char **argv) {
    try {
        std::cout << "argc = " << argc << std::endl;
        if (argc == 1) {

            std::cout << "FaceQuality port" << std::endl;
            std::cout << "For example" << std::endl;
            std::cout << "FaceQuality 8080" << std::endl;
            return 0;
        }

        int port = std::atoi(argv[1]);
        std::cout << "http server listen on port " << port << std::endl;


        api = new BaiduFaceApi();

        int ret = api->sdk_init();
        std::cout << "baidu face sdk init " << ret << std::endl;

        if (ret == 0) {


            WebServerApp app;
            int app_ret =  app.run(argc, argv);

            delete api;

            return app_ret;
        } else {
            delete api;
            return 0;
        }

    }
    catch (Poco::Exception &exc) {
        std::cerr << exc.displayText() << std::endl;
        return Poco::Util::Application::EXIT_SOFTWARE;
    }
}


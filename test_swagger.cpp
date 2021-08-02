#include <iostream>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

int main()
{
    //create  a file stream to write the received data into it
    auto fileStream = std::make_shared<ostream>();

    // Open stream to output file.
    pplx::task<void> requestTask = fstream::open_ostream(U("users.json"))
        
        //make a GET request
        .then([=](ostream outFile)
            {
                *fileStream = outFile;

                // Create http_client to send the request.
                http_client client(U("https://reqres.in/"));

                // Build request URI and start the request.
                uri_builder builder(U("api"));
                builder.append_path(U("users"));
                return client.request(methods::GET, builder.to_string());
            })

        // Handle response headers arriving.
        .then([=](http_response response)
            {
                //Check status code
                if (response.status_code() != 200)
                {
                    throw std::runtime_error("Returned " + std::to_string(response.status_code()));
                }
                else
                {
                    printf("Received response status code:%u\n", response.status_code());
                }

                // Write response body into the file.
                return response.body().read_to_end(fileStream->streambuf()).wait();
            })

        // Close the file stream.
        .then([=](size_t)
            {
                return fileStream->close();
            });
            
        // Wait for all the outstanding I/O to complete and handle any exceptions
        try
        {
            while (!requestTask.is_done())
            {
                std::cout << ".";
            }
        }
        catch (const std::exception& e)
        {
            printf("Error exception:%s\n", e.what());
        }

        return 0;
}
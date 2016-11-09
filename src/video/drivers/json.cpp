/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/video/video_factory.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/file_extension.h>
#include <pangolin/utils/transform.h>

#include <fstream>
#include <functional>

namespace pangolin {

PANGOLIN_REGISTER_FACTORY(JsonVideo)
{
    struct JsonVideoFactory : public VideoFactoryInterface {
        std::unique_ptr<VideoInterface> OpenVideo(const Uri& uri) override {
            if(uri.scheme == "json" || (uri.scheme == "file" && FileLowercaseExtention(uri.url) == ".json")) {
                const std::string json_filename = PathExpand(uri.url);
                std::ifstream f( json_filename );

                // Parse json file to determine sub-video
                if(f.is_open())
                {
                    json::value file_json(json::object_type,true);
                    const std::string err = json::parse(file_json,f);
                    if(err.empty())
                    {
                        // Json loaded. Parse output.
                        std::string input_uri = file_json.get_value<std::string>("video_uri", "");
                        if(!input_uri.empty())
                        {
                            // Transform input_uri based on sub args.
                            const json::value input_uri_params = file_json.get_value<json::object>("video_uri_defaults", json::object());
                            input_uri = Transform(input_uri, [&](const std::string& k) {
                                return uri.Get<std::string>(k,input_uri_params.get_value<std::string>(k,"#"));
                            });

                            return pangolin::OpenVideo(input_uri);
                        }else{
                            throw VideoException("JsonVideo failed.", "Bad input URI.");
                        }
                    }else{
                        throw VideoException("JsonVideo failed.", err);
                    }
                }else{
                    throw VideoException("JsonVideo failed. Unable to load file.", json_filename);
                }
            }else{
                // Not applicable for this factory.
                return std::unique_ptr<VideoInterface>();
            }
        }
    };

    auto factory = std::make_shared<JsonVideoFactory>();
    VideoFactoryRegistry::I().RegisterFactory(factory, 10, "json");
    VideoFactoryRegistry::I().RegisterFactory(factory,  5, "file");
}

}

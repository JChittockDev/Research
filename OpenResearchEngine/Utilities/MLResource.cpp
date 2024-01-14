#include "MLResource.h"

//void MLResource::CreateMLResources()
//{
//    std::ifstream json_file("filename");
//
//    std::vector<std::shared_ptr<SearlizedNode>> graph;
//
//    if (json_file.is_open())
//    {
//        json data;
//        json_file >> data;
//        
//        for (auto& input : data["graph"]["input"])
//        {
//            std::shared_ptr<SearlizedNode> inputNode = std::make_shared<SearlizedNode>(input["name"], data);
//            graph.push_back(inputNode);
//        }
//    }
//}
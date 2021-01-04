#ifndef IMG_GROUP
#define IMG_GROUP
#include <string>
#include <vector>

struct img_group
{
    std::string doc_id;
    std::string description;
    std::string upload_time;
    std::vector<std::string> imgs;
};
#endif
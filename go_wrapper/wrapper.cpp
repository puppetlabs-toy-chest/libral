#include <libral/export.h>
#include <libral/libral.hpp>
#include <libral/ral.hpp>
#include <libral/host.hpp>
#include <libral/emitter/puppet_emitter.hpp>
#include <libral/emitter/json_emitter.hpp>
#include <string>
#include <vector>


// #ifdef __cplusplus
// extern "C" {
// #endif

namespace lib = libral;

int get_all_hosts() {
    std::string type_name = "host";
    std::vector<std::string> data_dirs;
    auto ral = lib::ral::create(data_dirs);
    std::unique_ptr<lib::emitter> emp;
    // Use JSON emitter
    emp = std::unique_ptr<lib::emitter>(new lib::json_emitter());
    auto opt_type = ral->find_type(type_name);
    auto& type = *opt_type;

    auto insts = type->instances();
    lib::emitter& em = *emp;
    em.print_list(*type, insts);
    if (!insts) {
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    printf("Attempting to get all hosts\n");
    int returncode = get_all_hosts();
    return returncode;
}

// #ifdef __cplusplus
// }
// #endif
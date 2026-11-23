// setup.hpp - shared test fixtures (patients, catalogue, payers) for Tasks B1, B2, B4
#pragma once
#include "billing.hpp"
#include <fstream>

namespace hb {
inline PatientRegistry makeRegistry()
{
    PatientRegistry r;
    r.add("P101", "Ravi");
    r.add("P102", "Meena");
    r.add("P103", "Arun");
    r.add("P104", "Lakshmi");
    r.add("P105", "Farhan");
    return r;
}
inline PackageCatalogue makeCatalogue()
{
    PackageCatalogue c;
    c.add("CATARACT", 18000);
    c.add("HERNIA", 35000);
    c.add("DIALYSIS", 2500);
    return c;
}
inline GovernmentScheme makeScheme()
{
    return GovernmentScheme("CMCHIS", {{"OP", 1000},
                                       {"IP", 30000},
                                       {"DC-CATARACT", 15000},
                                       {"DC-DIALYSIS", 2500},
                                       {"ER", 5000}});
}
inline std::vector<std::string> readLines(const std::string& path)
{
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::vector<std::string> lines;
    for (std::string s; std::getline(in, s);)
        if (!s.empty() && s[0] != '#') lines.push_back(s);
    return lines;
}
}  // namespace hb

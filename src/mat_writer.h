#pragma once

#include <vector>
#include <tuple>
#include <ostream>
#include <string>

namespace mat
{
    struct header
    {};

    struct matrix
    {
        const std::string& name;
        const std::vector<std::pair<double, double>>& data;
    };

    std::ostream& operator<<(std::ostream& str, const header&);
    std::ostream& operator<<(std::ostream& str, const matrix& matrix);
}
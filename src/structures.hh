#pragma once

#include <vector>
#include <tuple>
#include <deque>

using graph_ty = std::vector<std::vector<bool>>;

// tuple<terrain, arbitre, team1, team2>
using match_ty = std::tuple<int, int, int, int>;
using match_list = std::vector<match_ty>;

using teams_ty = std::tuple<int, int, int>;

struct Team
{
    enum state {
        PLAY,
        SLEEP,
        JUDGE
    };

    Team(int n)
            : id{n}
    {}

    int id;
    std::deque<state> actions;

    template<state T>
    inline int count() const
    {
        auto res = 0;
        for (const auto& elm : actions)
            res += elm == T;
        return res;
    }

    inline void push(state t)
    {
        actions.push_back(t);
    }
};

using teams_list = std::vector<Team>;
using match_tup = std::vector<std::tuple<Team*, Team*, Team*>>;

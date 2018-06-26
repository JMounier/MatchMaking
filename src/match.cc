#include "match.hh"

#include <algorithm>

bool done(const graph_ty& g)
{
    for (const auto& elm : g)
        for(auto b : elm)
            if (!b)
                return false;
    return true;
}

int count_left(const graph_ty& g, int max)
{
    int res = 0;
    for (const auto& elm : g)
        for(auto b : elm)
            if (b)
            {
                res++;
                if (res == max)
                    return max;
            }
    return res;
}

std::vector<std::tuple<Team*, Team*>> list_remaining(const graph_ty& g, teams_list& teams)
{
    std::vector<std::tuple<Team*,Team*>> res;

    for (auto i = 0; i < g.size(); ++i)
        for(auto j = 0; j < i; ++j)
            if (!g[i][j])
                res.push_back(std::make_tuple(&teams[j], &teams[i]));

    std::sort(possible.begin(), possible.end(),
              [](const std::tuple<Team*,Team*>& a, const std::tuple<Team*,Team*>& b)
              {
                  int sum_a = a.get<0>()->count<Team::PLAY>() + a.get<1>()->count<Team::PLAY>();
                  int sum_b = b.get<0>()->count<Team::PLAY>() + b.get<1>()->count<Team::PLAY>();
                  if (sum_a != sum_b)
                      return sum_a > sum_b;
                  sum_a = a.get<0>()->count<Team::SLEEP>() + a.get<1>()->count<Team::SLEEP>();
                  sum_b = b.get<0>()->count<Team::SLEEP>() + b.get<1>()->count<Team::SLEEP>();
                  return sum_a < sum_b;
              });
    return res;
};

graph_ty gen_matches(int nb_team)
{
    graph_ty res;
    for (auto i = 0; i < nb_team; i++)
    {
        res.emplace_back();
        for (auto j = 0; j < nb_team; j++)
            res.back.emplace_back(j == i);
    }
    return res;
}

teams_list gen_teams(int nb_team)
{
    teams_list res;
    for (auto i = 0; i < nb_team; i++)
        res.push_back(Team(i));
    return res;
}

match_list make_match(int nb_team, int nb_field)
{
    auto graph = gen_matches(nb_team);

    teams_list teams = gen_teams(nb_team);
    std::vector<Team*> teams_cln = clone(teams);
    match_list res;

    int nb_round = 0;

    while (!done(graph))
    {
        gen_round(res, graph, teams_cln, nb_field);
        nb_round++;
        apply_sleep(teams, nb_round);
    }

    return res;
}

void apply_sleep(teams_list& teams, int num)
{
    for (auto& elm : teams)
        while (elm.actions.size() < num)
            elm.push(Team::SLEEP);
}

void gen_round(match_list& buf, graph_ty& graph, std::vector<Team*>& teams, int nb_field)
{
    const nb_match = graph.size() * graph.size() - graph.size();
    int max = count_left(graph, nb_field);

    auto possible = list_remaining(g);

    match_tup selected;

    std::sort(teams.begin(), teams.end(),
              [](const Team* a, const Team* b)
              {
                  int sum_a = a->count<Team::JUDGE>();
                  int sum_b = b->count<Team::JUDGE>();
                  if (sum_a != sum_b)
                      return sum_a < sum_b;
                  return a->count<Team::SLEEP>() < b->count<Team::SLEEP>();
              });

    for (auto i = 0; i < max; ++i)
    {
        bool found = false;

        for (auto it = possible.begin(); it != possible.end(); ++it)
            if (compatible(selected, it->get<0>()) && compatible(selected, it->get<1>()))
            {
                auto judge = find_judge(selected, *it, teams, nb_match);
                if (!judge)
                    continue;
                found = true;
                selected.push_back(std::make_tuple<>(judge, it->get<0>(), it->get<1>()));
                possible.erase(possible.being(), it + 1);
                break;
            }

        if (!found)
            break;
    }

    if (selected.size() == 0)
        throw std::runtime_error("cannot find any valid match to play");

    apply_selection(buf, graph, selected);
}

void apply_selection(match_list& buf, graph_ty& graph, match_tup& selected)
{
    int field = 0;
    for(const auto& elm : selected)
    {
        int j = elm.get<0>()->id;
        int t1 = elm.get<1>()->id;
        int t2 = elm.get<2>()->id;
        buf.push_back(std::make_tuple<>(field++, j, t1, t2));

        graph[t1][t2] = true;
        graph[t2][t1] = true;
        elm.get<0>()->push(Team::JUDGE);
        elm.get<1>()->push(Team::PLAY);
        elm.get<2>()->push(Team::PLAY);
    }
}

Team* find_judge(const match_tup& selected, const std::tuple<Team*, Team*>& match,
                 std::vector<Team*>& teams, int nb_match)
{
    int max = nb_match / teams.size() + 1;

    for (auto elm : teams)
        if (elm->count<Team::JUDGE>() < max && match.get<0>() != elm && match.get<1>() != elm)
            if (compatible(selected, elm))
                return elm;

    return nullptr;
}

bool compatible(const match_tup& selected,const Team* team)
{
    return std::any_of(selected.cbegin(), selected.cend(),
                       [&team](const std::tuple<Team*, Team*, Team*> elm)
                       {
                           return elm.get<0>() == team || elm.get<1>() == team || elm.get<2>() == team;
                       });
}

std::ostream& operator<<(std::ostream&, const match_list&)
{
    out << "matches:" << std::endl;
    int last = -1;
    int round = 0;
    for (auto it = list.cbegin(); it != list.cend(); it++)
    {
        if (it->get<0>() <= last)
        {
            out << "ROUND N°" << round << std::endl;
            round++;
        }
        last = it->get<0>();
        out << "terrain: " << last;
        out << ",\"arbitre\":" << it->get<1>();
        out <<",\"equipes\":";
        out << "[" << it->get<2>() << "," << it->get<3>() << "]" << std::endl;
    }
}
void dump(std::ostream& out, const match_list& list)
{
    out << "{\"matches\":[";
    for (auto it = list.cbegin(); it != list.cend(); it++)
    {
        if (it != list.cbegin())
            out << ',';
        out << "{\"terrain\":" << it->get<0>();
        out << ",\"arbitre\":" << it->get<1>();
        out <<",\"equipes\":";
        out << "[" << it->get<2>() << "," << it->get<3>() << "]}";
    }

    return out << "]}";
}

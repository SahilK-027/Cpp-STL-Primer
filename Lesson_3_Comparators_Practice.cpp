/*

// P1. vector<pair<string,int>> of {name, score}. Sort by score high to low; equal scores by name A to Z. Input: {"bob",90}, {"amy",95}, {"cat",90}, {"dan",80} Expected: amy 95, bob 90, cat 90, dan 80
auto cmp = [](const pair<string,int>& a, const pair<string,int>& b){
    if(a.second != b.second) return a.second > b.second;
    return  a.first < b.first;
};
sort(v.begin(), v.end(), cmp);


// P2. Push 7, 2, 9, 4 into a priority_queue<int> so that popping everything prints them smallest first. Expected: 2 4 7 9
priority_queue<int, vector<int>, greater<int>>;


// P3. A set<int> that stores elements in descending order. Insert 5, 1, 9, 5 and print it. Expected: 9 5 1
auto cmp = [](int a, int b){
    return a > b;
};
set<int, decltype(cmp)> s(cmp);


// P4. Sort words by length, shortest first; equal lengths alphabetically. Input: "pear", "fig", "apple", "kiwi", "date" Expected: fig date kiwi pear apple
auto cmp = [](const string& a, const string& b){
    if(a.size() != b.size()) return a.size() < b.size();

    return a < b;
};
sort(v.begin(), v.end(), cmp);


// P5. struct Task { string name; int deadline, profit; }; A priority_queue<Task> that pops the highest profit first; equal profits, earliest deadline first. Input: {"A",2,50}, {"B",1,50}, {"C",3,80}, {"D",1,20} Expected pop order: C B A D

auto cmp = [](const Task& task1, const Task& task2){
    if (task1.profit != task2.profit) return task1.profit < task2.profit;
    return task1.deadline > task2.deadline;
};
priority_queue<Task, vector<Task>, decltype(cmp)> pq(cmp);


// P6. vector<int> a = {30, 10, 20, 10}; Build the list of indices sorted by value high to low; equal values, smaller index first. Don't change a. Expected: 0 2 1 3
vector<int> idx(a.size()); // 4 slots
iota(idx.begin(), idx.end(), 0) // idx = {0, 1, 2, 3}

auto cmp = [&](int i, int j){
    if (a[i] != a[j]) return a[i] > a[j];
    return i < j;
};
sort(idx.begin(), idx.end(), cmp);


// P7. vector<int> d = {50, 40, 30, 20, 10}; (sorted descending). With lower_bound, find the first element that is 35 or less. Expected: value 30 at index 2
lower_bound(d.begin(), d.end(), 35, greater<int>());   // 30, index = it - d.begin() = 2


// P8. Sort integers by absolute value; for equal absolute values, the negative one first. Input: 3, -1, -3, 2, 1 Expected: -1 1 2 -3 3
auto cmp = [](int a, int b){
    if (abs(a) != abs(b)) return abs(a) < abs(b);
    return a < b;
};
sort(arr.begin(), arr.end(), cmp);


// P9. A map<string,int> whose keys are kept in reverse alphabetical order. Insert apple, cherry, banana and print the keys. Expected: cherry banana apple
auto cmp = [](const string& a, const string& b){
    return a > b;
};
map<string,int, decltype(cmp)> mp(cmp);


// P10. Events as pair<int,int> {time, count}. A priority_queue that pops the earliest time first; for equal times, the larger count first. Solve it twice: once with a comparator, once with the negation trick and greater. Input: {3,5}, {1,2}, {3,9}, {1,7} Expected pop order: {1,7} {1,2} {3,9} {3,5}
auto cmp = [](const auto& a, const auto& b){
    if (a.first != b.first) return a.first > b.first;
    return a.second < b.second;
};
priority_queue<pair<int,int>, vector<pair<int,int>>, decltype(cmp)> pq(cmp);


// P11. vector<Job> (struct Job { int deadline, profit; };) sorted by deadline ascending, with deadlines 1, 3, 4, 4, 6, 8. With upper_bound and a comparator, find the first job whose deadline is greater than 4. Expected: deadline 6 at index 4
struct Job { int deadline, profit; };
auto cmp = [](int x, const Job& j){
    return x < j.deadline;
};
upper_bound(v.begin(), v.end(), 4, cmp);


// P12. vector<Job> (struct Job { int deadline, profit; };) sorted by deadline ascending, with deadlines 1, 3, 4, 4, 6, 8. With lower_bound and a comparator, find the first job whose deadline is the first job with deadline ≥ 4
struct Job { int deadline, profit; };
auto cmp = [](const Job& j, int x){
    return j.deadline < x;
};
lower_bound(v.begin(), v.end(), 4, cmp);

*/
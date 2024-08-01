#include "cs50.h"
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
    int strength;
    bool remove;
} pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;
int visit[MAX] = {0};           // 0 = unvisited | 1 = visiting | 2 = visited
string direction[MAX];          // set if the direction is winning or losing
bool neighbors[MAX][MAX] = {0}; // neighbors of a candidate (if they (the first bracket) won)

bool unvisited[MAX] = {0};
bool visiting[MAX] = {0};
bool visited1[MAX] = {0};

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);
void cycle_check(int);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    // Establishing values for visiting

    for (int i = 0; i < candidate_count; i++)
    {
        unvisited[i] = 1;
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(name, candidates[i]) == 0)
        {
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            if (preferences[i][j] > preferences[j][i])
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pairs[pair_count].strength = preferences[i][j] - preferences[j][i];
                pairs[pair_count].remove = 0;
                neighbors[i][j] = 1; // establishing j is a neighbor of i (i can get to j)
                pair_count++;
            }
            else if (preferences[i][j] < preferences[j][i])
            {
                pairs[pair_count].winner = j;
                pairs[pair_count].loser = i;
                pairs[pair_count].strength = preferences[j][i] - preferences[i][j];
                pairs[pair_count].remove = 0;
                neighbors[j][i] = 1; // establishing i is a neighbor of j (j can get to i)
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    for (int i = 0; i < pair_count - 1; i++)
    {
        for (int j = 0; j < pair_count - 1 - i; j++)
        {
            if (preferences[pairs[j + 1].winner][pairs[j + 1].loser] >
                preferences[pairs[j].winner][pairs[j].loser])
            {
                pair temp = pairs[j];
                pairs[j] = pairs[j + 1];
                pairs[j + 1] = temp;
            }
        }
    }
    return;
}

bool dfs_cycle(int candidate, bool visited[], bool inStack[])
{
    if (inStack[candidate])
        return true;

    if (visited[candidate])
        return false;

    visited[candidate] = true;
    inStack[candidate] = true;

    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[candidate][i] && dfs_cycle(i, visited, inStack))
            // there is a cycle
            return true;
    }
    // no cycle
    inStack[candidate] = false;
    return false;
}

bool has_cycle()
{
    bool visited[MAX] = {0};
    bool inStack[MAX] = {0};

    for (int i = 0; i < candidate_count; i++)
    {
        if (!visited[i] && dfs_cycle(i, visited, inStack))
        {
            // found a cycle
            return true;
        }
    }
    // No cycle found
    return false;
}

void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++)
    {
        // temporarily lock pair
        locked[pairs[i].winner][pairs[i].loser] = true;
        // checks for
        if (has_cycle())
        {
            // unlocks pair
            locked[pairs[i].winner][pairs[i].loser] = false;
        }
    }
}

// Print the winner of the election
void print_winner(void)
{
    bool loser[MAX] = {0};
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[i][j])
                loser[j] = 1;
        }
    }
    for (int i = 0; i < candidate_count; i++)
    {
        if (!loser[i])
        {
            printf("%s\n", candidates[i]);
            return;
        }
    }
}
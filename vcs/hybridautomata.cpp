#include "hybridautomata.h"

HybridAutomata::HybridAutomata(const unsigned int init, const unsigned int exit)
{
    if (exit >= MAX_STATES)
    {
        cout << "State is bigger than MAX_STATE" << endl;
        return;
    }
    else if (init < 0)
    {
        cout << "State should be unsigned" << endl;
        return;
    }
    initState = init;
    curState = initState;
    exitState = exit;

    initStateMachineArr();
    initStateArr();
    initConditionsArr();

    nStates = 2;
}
HybridAutomata::HybridAutomata()
{
    initState = 0;
    curState = initState;
    exitState = MAX_STATES;

    initStateMachineArr();
    initStateArr();
    initConditionsArr();

    nStates = 2;
}
void HybridAutomata::initStateArr()
{
    for (int i = 0; i < MAX_STATES; i++)
    {
        states[i] = NULL;
    }
}
void HybridAutomata::initStateMachineArr()
{
    for (int i = 0; i < MAX_STATES; i++)
    {
        for (int j = 0; j < MAX_STATES; j++)
        {
            stateMachine[i][j] = 0;
        }
    }
}
void HybridAutomata::initConditionsArr()
{
    for (int i = 0; i < MAX_STATES; i++)
    {
        for (int j = 0; j < MAX_STATES; j++)
        {
            conditions[i][j] = NULL;
        }
    }
}

void HybridAutomata::setState(unsigned int id, void (*ah)())
{
    if (id > exitState)
    {
        cout << "State id is bigger than exit State" << endl;
    }
    else if(nStates>MAX_STATES)
    {
        cout<< "State maximum" <<endl;
        return;
    }
    states[id] = new State(id, ah);
    nStates++;
}

void HybridAutomata::setCondition(unsigned int preState, Condition *cDo, unsigned int postState)
{
    if (preState > exitState || postState > exitState || preState < initState || postState < initState)
    {
        cout << "StateId error in setCondition func" << endl;
        return;
    }

    bool isEmpty;
    isEmpty = checkStateMachine(preState, postState);
    if (isEmpty == true)
    {
        stateMachine[preState][postState] = 1;
        conditions[preState][postState] = cDo;
    }
    else
        cout << "this condition has already declared" << endl;
}
void HybridAutomata::operate()
{
   //cout<<"inoperate func beforeState : "<<curState<<endl;
   curState = checkConditions();
   //if(curState == exitState) return;
   //cout<<"Condition satisfied! curState = "<<curState<<endl;
   states[curState]->aDo();
}
bool HybridAutomata::checkStateMachine(unsigned int pre, unsigned int post)
{
    if (stateMachine[pre][post] == 0)
        return true;
    else
        return false;
}
/*
bool HybridAutomata::checkConditions(unsigned int post)
{
    bool isAvail = false;
    if (checkStateMachine(curState, post) == false)
    {
        curState=post;
        Condition *cand = conditions[curState][post];

        if (cand == NULL)
            return true;
        isAvail = cand->check(this);
        if (isAvail == true)
            return true;
    }
    return false;
}
*/
int HybridAutomata::checkConditions()
{
    
   bool isEmpty = true;
   bool isAvail = false;
   int idx = 0;
   int temp = 0;
   Condition *cand;
   int candidateArr[MAX_STATES];
   for (int i = initState; i<=exitState; i++)
   {
      isEmpty = checkStateMachine(curState, i);
      if (isEmpty == false)
      {
         candidateArr[idx] = i;
         idx++;
      }
   }
   if(idx == 0 && curState != exitState)
   {
       printf("[HybridAutomata] No post State, curState = %d\n",curState);
       return exitState;
   }
   while (true)
   {
      temp = temp % idx;
      
      cand = conditions[curState][candidateArr[temp]];
      //printf("cand : %d\n",candidateArr[temp]);
      if(cand == NULL) return candidateArr[temp];
      isAvail = cand->check(this);
      if (isAvail == true) return candidateArr[temp];

      ++temp;
   }
/*
   int candNum = 0;
   bool isEmpty;
   bool isAvail = false;
   int candidateArr[MAX_STATES];
   Condition *cand;
   for (int i = initState; i <= exitState; i++)
   {
       isEmpty = checkStateMachine(curState, i);
       if (isEmpty == false)
       {
           candNum++;
           cand = conditions[curState][i];
           if (cand == NULL)
               return i;
           isAvail = cand->check(this);
           if (isAvail == true)
               return i;
       }
   }
   if(candNum==0) printf("[HybridAutomata] No Conditions\n");
   printf("There is no Candidate State!\n");
   return curState;
  */ 

}
HybridAutomata::~HybridAutomata()
{
    cout << "~HybridAutomata called" << endl;
    for (int i = initState; i < exitState; i++)
    {
        for (int j = initState; j < exitState; j++)
        {
            if (conditions[i][j] != NULL)
                delete conditions[i][j];
        }
        if (states[i] != NULL)
            delete states[i];
    }
}

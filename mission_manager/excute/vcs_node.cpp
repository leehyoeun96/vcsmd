#include <iostream>
#include <stdlib.h>

int main()
{
	system("gnome-terminal -x sh -c 'rosrun vcs_con vcs_con bash'");
	system("gnome-terminal -x sh -c 'rosrun vcs_agent vcs_agent bash'");
	system("gnome-terminal -x sh -c 'rosrun vcs_mon vcs_mon 10; bash'");
	return 0;
}

#include <dirent.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include "/home/pi/programowanie/iDom_server/src/files_tree/files_tree.h"

int main( int argc, char ** argv ) {
    char choise;
    bool go_while=true;
	std::string s(argv[1]);
    files_tree main_tree( s);
    //main_tree.show_list();
   
    while (go_while) {
     
           
            std::cout << " wybierz " << std::endl;
            std::cin >> choise;
            if (choise =='n')
            {    
				main_tree.next(  );
		      
            }
			else if ( choise =='p' )
            {   
				main_tree.previous( );
			 
            }
            else if ( choise =='t' )
            {
                std::cout << " URUCHAMIAM PLIK! " << std::endl;
				 main_tree.enter_dir();
		
                continue;
            }
            
		 
            else if ( choise =='u')
            {
                	main_tree.back_dir();	 
                continue;
            }
            else if ( choise =='e')
            {
                go_while=false;
                break;
            }
       // } //end for
    }  // end while
    return 0;
}

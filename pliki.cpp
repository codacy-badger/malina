#include "files_tree.h"



main( int argc, char ** argv ) {
char choise;
bool go_while=true;
movies_tree main_tree( argv[1]);
//main_tree.show_list();
while (go_while) {
if ( choise =='b'){
main_tree.get_main_list();
}
std::cout << " rozmiar vecotra : " << main_tree.get_vector_size() << std::endl;
for( size_t i = 0; i < main_tree.get_vector_size(); i++ )
   {
   main_tree.list_tree( i );
   std::cout << "iteracja: " << i << std::endl;
   std::cin >> choise;
   if (choise =='n')
        {
        continue;
        }
        else if ( choise =='t' &&  main_tree.is_file(i) == true)
        {
          std::cout << " URUCHAMIAM PLIK! " << std::endl;
          break;
        }
        else if ( choise =='t' &&  main_tree.is_file(i)== false)
        {
            main_tree.enter_dir( i );
            std::cout << " rozmiar vecotra : " << main_tree.get_vector_size() << std::endl;
            continue;
        }
         else if ( choise =='b')
          {
            main_tree.vector_clear();
           break;
          }
           else if ( choise =='e')
          {
            go_while=false;
            break;
          }
        } //end for
    }  // end while
return 0;
}
/*
movie_database temp;
char choise;
bool go_while = true;
while (go_while)
{
   if( argc > 1 )
         listujPliki( argv[ 1 ],&movie_database_vector );
    else
         listujPliki( "." ,&movie_database_vector);

   std::cout << " teraz wypisze vektor " << movie_database_vector.size() << std::endl;

   for( size_t i = 0; i < movie_database_vector.size(); i++ )
   {
        if (movie_database_vector[i].is_file == true ) {
        std::cout << "wypisuje sciezke pliku " << movie_database_vector[i].path << std::endl;
        std::cout << "uruchomic? " << std::endl;
        std::cin >> choise;
        }
        else {
           std::cout << "wypisuje sciezke katalogu " << movie_database_vector[i].path << std::endl;
           std::cout << "wejsc? " << std::endl;
        std::cin >> choise;
        }

        if (choise =='n')
        {
        continue;
        }
        else if ( choise =='t' &&  movie_database_vector[i].is_file == true)
        {
          std::cout << " URUCHAMIAM PLIK! " << std::endl;
          break;
        }
          else if ( choise =='t' &&  movie_database_vector[i].is_file == false)
          {
            listujPliki( movie_database_vector[i].path ,&movie_database_vector);
          }
          else if ( choise =='b')
          {
            movie_database_vector.clear();
           break;
          }
           else if ( choise =='e')
          {
            go_while=false;
            break;
          }
   } //end for

   } // end while
   return 0;
} */

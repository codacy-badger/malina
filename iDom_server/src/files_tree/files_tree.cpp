#include "files_tree.h"


files_tree::files_tree (std::string path, LCD_c *mainLCD_PTR):w_serial( "([Ss]\\d{1,3}[Ee]\\d{1,3})")
{
    database_path = path;
    mainLCD=mainLCD_PTR;
	//tree_stack.push(database_path);
	i_stack.push(0);
	i=0;
     get_list( database_path  );

}
bool files_tree::is_file(  )
{
    return movie_database_vector[i].is_file;
}
std::string files_tree::return_path( int i)
{
    std::cout << " return " << movie_database_vector[i].path <<std::endl;
    return movie_database_vector[i].path;
}

void files_tree::get_main_list()
{
    get_list( database_path );
}
void files_tree::next(  )
{   
      ++i;
	//std::cout << " dodaje " << i << std::endl;
    if (get_vector_size() == i ){
       i=0;
    }
   	 	std::cout << " iteracja i ma : " << i <<" rozmiar vectora : " << get_vector_size() <<  std::endl;
     // show_list();
 
}

void files_tree::previous(  )
{
    --i;
	//std::cout << " odejmuje " << i << std::endl;
	
	if ( i <0 )
	{
       i=get_vector_size()-1;   
    }
		std::cout << " iteracja i ma : " << i << " rozmiar vectora : " << get_vector_size() <<std::endl;
	//show_list();
}

int files_tree::get_vector_size ()
{
    return  movie_database_vector.size();
}
void files_tree::vector_clear ()
{       
         movie_database_vector.clear();
}
 int files_tree::get_i()
 { if (i_stack.size() >1 ) {
	int i = i_stack.top();
	i_stack.pop();
	return i;
	}
	else
	return 0;
 }

void files_tree::enter_dir()
{
   i_stack.push(i);  // wpisuje na stos  kolejna wersje licznika i 
    
 
    if (movie_database_vector[i].is_file == true ) {
        std::cout << "to nie KATALOG!"<< std::endl;

    }
    else {

        std::cout << "sciezka: " << movie_database_vector[i].path << " i i: " << i << std::endl;
        std::cout << " WHODZE W TO!" << std::endl;
		tree_stack.push(movie_database_vector[i].path);
		 std::cout << " w kolejce jest elementow : "<< tree_stack.size()<< std::endl;
		 std::cout << " a ostatni to : "<< tree_stack.top()<< std::endl;
        get_list (movie_database_vector[i].path)  ;
			  i=0;
    }

}
void files_tree::enter_dir(std::string path)
{	tree_stack.push(path);
      get_list (path)  ;

}

void files_tree::back_dir()
{
		if ( tree_stack.size() >1 ) {
		tree_stack.pop();
		std::string path = tree_stack.top();
		tree_stack.pop();
        std::cout << "                                           robie back do folderu: " << path << std::endl;
		 std::cout << " w kolejce jest TERAZ elementow : "<< tree_stack.size()<< std::endl;
        tree_stack.push(path);
			i=get_i();
			
			 std::cout << "    III ma teraz : " << i << std::endl;
		get_list (path)  ;
		return;
		}
     	i=get_i();
		
			 std::cout << "    III ma teraz : " << i << std::endl;
    get_list (  database_path);
	return;
}

std::string files_tree::show_list(     ) 
{
     	std::cout << "iteracja!!!!!!!!!!!!!!!!!!!!!!: " << i <<"  rozmiar vectora : " << get_vector_size() << std::endl;
		 if (movie_database_vector[i].is_file == true ) {
             std::cout << "wypisuje sciezke pliku " << movie_database_vector[i].path <<" | " <<movie_database_vector[i].files_name.substr(0,16)<< std::endl;

            mainLCD->printString(true,0,0,movie_database_vector[i].files_name.substr(0,16));
            if( regex_search(movie_database_vector[i].path,result,w_serial) )
            {
                mainLCD->printString(false,10,1,movie_database_vector[i].files_name.substr( movie_database_vector[i].files_name.size()-4,movie_database_vector[i].files_name.size()));
                mainLCD->printString(false, 1,1,result[0]);
                std::cout << " SERIAL!!!!!!!!!!!!!!!!!!!!!!!!!!1" << result[0] << std::endl;
            }
            else
            {
                mainLCD->printString(false,10,1,movie_database_vector[i].files_name.substr( movie_database_vector[i].files_name.size()-4,movie_database_vector[i].files_name.size()));
               std::cout << " NIEEEEEE SERIAL!!!!!!!!!!!!!!!!!!!!!!!!!!1" << std::endl;
            }


        }
        else {
            std::cout << "wypisuje sciezke katalogu " << movie_database_vector[i].path << std::endl;
            mainLCD->printString(true,0,0,movie_database_vector[i].files_name+"/");
        }

		return movie_database_vector[i].path;

}


void files_tree::get_list( std::string path  ) {
    //tree_stack.push(path);
    vector_clear();    // czyscimy vector
  //  i=0;
   std::string  path2 =path;
   std::string v_path ,tmp_string ;
    std::cout << " SCIEZKA TO " << path << std::endl;
    //std::cin >> g;

    if(sciezka = opendir( path.c_str() ))  {


        while(( plik = readdir( sciezka ) ) )
        {
				path2 =path;
         
			//	std::cout << " SCIEZKA TO " << path << std::endl;
            if (  static_cast<int>(plik->d_type) == 4 /*&&   strcmp( plik->d_name, ".." ) && strcmp( plik->d_name, "." )*/  )
            {

                temp.is_file=false;
             

            }
           else //if ( (int)plik->d_type == 8 && strcmp( plik->d_name, ".." ) && strcmp( plik->d_name, "." ))
            {
               temp.is_file=true;
                 
           }
          
		    v_path= path2;
			
			v_path+="/";
		
			tmp_string.assign(plik->d_name);
		    v_path+=tmp_string;
		   temp.path =v_path;
           temp.files_name.assign(plik->d_name);
		   
           // std::cout << "wpisuje w strukture " << plik->d_name << std::endl;
           
			//std::cout << "\n";
			 
			
           // std::cout << "wpisuje w vector " << temp.path << " i jest to " << temp.is_file << std::endl;
            movie_database_vector.push_back(temp);

        }  // end while

        closedir( sciezka );
    }
    else
        std::cout << " wywoujc funkcj opendir(%s) pojawi si bd otwarcia strumienia dla danej cieki, moe nie istnieje, lub podano ciek pust\n"<<  path << std::endl;

}

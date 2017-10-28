#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <assert.h>

void fatalError( const char *s){
    perror( s );
    exit( 1 );
}

void    doInsertion( int insertIndex, char *argv[] ){
    puts( "inside doInsertion");
    int in = *argv[insertIndex] == '<';//checks direction 
    int flags = in ? O_RDONLY : (O_WRONLY|O_CREAT|O_TRUNC);
    int fdIndex = in ? 0 : 1;
    
    argv[insertIndex] = NULL;
    const char *path = argv[ insertIndex + 1 ];
    int fd; 
    
    //mode 666(hail satan) in decimal 438 
    const int RWRWRW = 438;
    fd = open( path , flags , RWRWRW );
    assert( fd > 2 );
    if( dup2( fd , fdIndex ) == -1 )
        fatalError(" > dup2 problems");
    if( close( fd ) == -1 )
        fatalError( "problems closing 0 and 1 in insert" );
    puts("inside doInsertion this will show up where output file" );
}

void setupRedirect( char **cmds[] , int job ){
    for ( char **arg = cmds[job] ; arg != NULL ; arg++ ){
        if( **arg == '>' || **arg == '<' ){
            printf( " inside insertion\n");
            doInsertion( ( arg - cmds[job] ) , cmds[job] );
        }
    }
}

void cmdHandler( int argc, char *argv[] ){
    char **cmds[argc];
    enum { PIPE0IN , PIPE0OUT , PIPE1IN , PIPE1OUT };
    int jobcount = 0;
    cmds[jobcount++] = &argv[0];
    bool isRedirect = false;

    for( int i = 0 ; i < argc ; i++ ){
        if( *argv[i] == '<' || *argv[i] == '>'){ 
            isRedirect = true;
        }
        if( *argv[i] == '|' ){
            argv[i] = NULL;
            cmds[jobcount++] = &argv[ i + 1 ];
        }
    }
    size_t pipecount = ( jobcount - 1 );
    int pipes[pipecount*2];
    for( int i = 0 ; i < pipecount ; i++ ){
        printf( "i: %d offset: %d\n" , i, 2*i);
        pipe(pipes + 2*i);
    }
        

    for( int job = 0 ; job < jobcount ; job++ ){
       
        switch( fork() ){
        case -1: fatalError( "bad fork " );
        case 0: 
            if( isRedirect ){
                setupRedirect( cmds, job );
            }
            // if there is only one job will not try to pipe
            if( jobcount > 1 ){
                if ( job == 0 ) {
                    if( dup2( pipes[PIPE0OUT] , STDOUT_FILENO ) == -1)
                        fatalError("bad dup2 on first job");
                }
                else if ( job == ( jobcount - 1 ) ){
                    int a = (job - 1)*2;
                    printf( "(job-1)*2 %d\n jobcount %d\n job %d\n pipes[a] %d\n " , a , jobcount , job , pipes[a]);
                    if(dup2( pipes[ a ], STDIN_FILENO ) == -1)
                        fatalError("bad dup2 on last job");
                        
                }
                else{
                    if(dup2( pipes[(job - 1)*2] , STDIN_FILENO ) == -1)
                        fatalError("middle pipe failed");
                    if(dup2( pipes[(job*2) + 1 ], STDOUT_FILENO ) == -1)
                        fatalError("middle pipe failed");
                }
            }        
            for( int i = 0 ; i < pipecount ; i++ )
                close( pipes[i] );
            
            execvp( *cmds[job] , cmds[job] );
        default:  
            for( int i = 0 ; i < pipecount ; i++ )
                close( pipes[i] );
            
        }
   
    }
    while( wait(NULL) != -1 );
}


int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    printf( "Enter Command:\n" );
    
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);
        cmdHandler( num_words, line_words );
        printf( "Enter Command:\n" );
        
    }
    

    return 0;
}


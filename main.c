#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

void fatalError( const char *s){
    perror( s );
    exit( 1 );
}
// making this function void unlike others
void    doInsertion( int insertIndex, char *argv[] ){
    pid_t pid;
    //use ternary operator to pick > or < 
    int in = *argv[insertIndex] == '<';
    int mode = in ? O_RDONLY : (O_WRONLY|O_CREAT|O_TRUNC);
    int fdIndex = in ? 0 : 1;
    argv[insertIndex] = NULL;
    const char *path = argv[ insertIndex + 1 ];
    int fd; 
    switch ( pid = fork() ){
        case -1:
            fatalError("< >  fork failed");
        case 0: 
            fd = open( path , mode );
            if( dup2( fd , fdIndex ) == -1 )
                fatalError(" > dup2 problems");
            if( close( fd ) == -1 )
                fatalError( "problems closing 0 and 1 in insert" );
            execvp( argv[0], argv );
            fatalError( "problems with insertR left opperand" );
    }


    
    while( wait(NULL) != -1 );
}
// may need extra file descriptor swap 
//after fork + exec(1st grep) need to ...
// where is that file descriptor sitting then 
// do some changes

//who outputs the no such file or directory

//TODO pass doPipe num words for argument parsing
//TODO use return value from pipe to indicate where 
//we are in the parsing
int doPipe( int pipeIndex, char *argv[]){
    int pfd[4];
    pid_t pid;
    argv[pipeIndex] = NULL;
    char **rOperand = &argv[ pipeIndex + 1 ];
    
    if ( pipe(pfd) == -1 )
        fatalError( "first pipe setup failed" );
    if( pipe( pfd + 2) == -1 )
        fatalError("second pipe problem");

    switch ( pid = fork() ){
        case -1:
            fatalError( "stdin fork");
        case 0:
        //what does the integer in pfd mean
            if ( dup2( pfd[0], STDIN_FILENO ) == -1 )
                fatalError( "failed to dup pfd[0]" );
            if( close( pfd[0] ) == -1 || close( pfd[1] ) == -1 )
                fatalError( "problems closing pfd[0] || prd[1]" );
            execvp( rOperand[0], rOperand );
            fatalError( "rOperand execvp failed" );

    }

    switch ( pid = fork() ){
        case -1:
            fatalError( "fork problemz!" );
        case 0:
            if ( dup2( pfd[1], STDOUT_FILENO ) == -1 )
                fatalError( " attaching pipe as stdout in child failed " );  
            if ( close( pfd[0] ) == -1 || close ( pfd[1] ) == -1 )
                fatalError( " problems closing pfd ");
            execvp( argv[0] , argv );
            fatalError( "could not execvp " );             
    }

    if( close( pfd[0] ) == -1 || close( pfd[1] ) == -1 )
        fatalError( "problem closing pfd in master process ");

    while( wait(NULL) != -1 );
    return 1;
}

int pipeHandler( int argc, char *argv[] ){
    char **cmds[argc];
    enum { PIPE0IN , PIPE0OUT , PIPE1IN , PIPE1OUT };
    int j = 0;
    cmds[j++] = &argv[0];
    for( int i = 0 ; i < argc ; i ++ ){
        if( *argv[i] == '|' ){
            argv[i] = NULL;
            cmds[j++] = &argv[ i + 1 ];
        
        }
    }

    //for( int i = 0; i < j ; i++){
    //    printf( "%s\n" , *cmds[i] );
    //}

    int pipes[4];
    pipe(pipes);
    pipe(pipes + 2);
    int k = 0;
    for( int job = 0 ; job < j ; job++ ){
        if( fork() == 0 ){
            //NeeDs to be gneralized
            switch(job) {
                case 0 :
                    dup2( pipes[PIPE0OUT] , STDOUT_FILENO );
                    break;
                case 1 : 
                    dup2( pipes[PIPE0IN] , STDIN_FILENO );
                    dup2( pipes[PIPE1OUT], STDOUT_FILENO );
                    break;
                case 2 : 
                    dup2( pipes[PIPE1IN], STDIN_FILENO );
                    break;
                default : fatalError( "KILLER BUNNY AHEAD" );
            
            }

            //TODO  instead of 4 need number of pipes 
            for( int i = 0 ; i < 4 ; i++ )
                close( pipes[i] );
            
            execvp( *cmds[job] , cmds[job] );
            
    
        }
    }
    //[ ls | grep test.c | wc ]
    // pipeIndex[ 1 , 4 ]
    for( int i = 0 ; i < 3 ; i ++){
         wait(NULL); 
    }
    return 0;
        
}


//you may want to have wait
#if 0
int doCommand( int argc, char *argv[]){
    pid_t pid;
    for( int i = 0 ; i < argc ; i++ ){
        if( *argv[i] == '|')
            return doPipe( i , argv ); 
        else if( *argv[i] == '>' || *argv[i] == '<' ){
            doInsertion( i , argv );  
            //TODO change doInsertion to return like do Pipe
            return 0;          
        }
    }
    switch ( pid = fork() ){
        case -1:
            fatalError( "doCommand fork failed" );
        case 0:
        //TODO fix line below and add to before doPipe exec
            printf( argv[0] );
            execvp( argv[0], argv );
            fatalError( "execvp doCommand failed" );
    }
    while( wait(NULL) != -1 );

    return 1; 

}
#endif

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);
        printf( "None Shall PASS, w/o cmmd " );
        pipeHandler( num_words, line_words );
        
    }

    return 0;
}


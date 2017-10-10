#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

void fatalError( const char *s){
    perror( s );
    exit( 1 );
}

int doPipe( int pipeIndex, char *argv[] ){
    int pfd[2];
    pid_t pid;
    argv[pipeIndex] = NULL;
    char **c2argv = &argv[ pipeIndex + 1];
    
    if ( pipe(pfd) == -1 )
        fatalError( "Nope on that pipe son" );
 
    switch ( pid = fork() ){
        case -1:
            fatalError( " doing second fork ");
        case 0:
            if ( dup2( pfd[0], STDIN_FILENO ) == -1 )
                fatalError( " attaching pipe as stdin in child failed " );
            if( close( pfd[0] ) == -1 || close( pfd[1] ) == -1 )
                fatalError( "problems closing child 2 pfd's" );
            execvp( c2argv[0], c2argv );
            fatalError( "c2argv execvp failed" );

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

int doCommand( int argc, char *argv[]){
    pid_t pid;
    for( int i = 0 ; i < argc ; i++ ){
        if( *argv[i] == '|')
            return doPipe( i , argv ); 
    }
    switch ( pid = fork() ){
        case -1:
            fatalError( "First fork failed" );
        case 0:
            execvp( argv[0], argv );
            fatalError( "could not doCommand" );
    }
    while( wait(NULL) != -1 );

    return 1; 

}

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];

    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);
        
        doCommand( num_words, line_words );
        
    }

    return 0;
}


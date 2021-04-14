import java.io.IOException;

import parse.Parser;

/**
 * Main GIS Class
 * 
 * @author Paul F Kim
 * @version 04.09.2017
 */
public class GIS {

    /**
     * Main function that runs GISparser
     * 
     * @param args
     *            args[0] = database file name, args[1] = command script file
     *            name, args[2] = log file name
     * @throws IOException 
     */
    public static void main(String[] args) throws IOException {

        Parser parse = new Parser();
        
        if(args.length != 3)
            System.out.println("Invalid command");
        else {
            
            parse.Parsing(args[0], args[1], args[2]);
        }
    }
}

//On my honor:
//
//- I have not discussed the Java language code in my program with
//anyone other than my instructor or the teaching assistants
//assigned to this course.
//
//- I have not used Java language code obtained from another student,
//or any other unauthorized source, either modified or unmodified.
//
//- If any Java language code or documentation used in my program
//was obtained from another source, such as a text book or course
//notes, that has been clearly noted with a proper citation in
//the comments of my program.
//
//- I have not designed this program in such a way as to defeat or
//interfere with the normal operation of the Curator System.
//
//Paul Fitzgerald Kim
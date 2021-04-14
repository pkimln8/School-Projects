package parse;

import hash.HashTable;
import quadTree.Point;
import quadTree.prQuadTree;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.Scanner;

/**
 * Parsing Class
 * 
 * @author Paul F Kim
 * @version 04.11.2017
 *
 */
public class Parser {

    private Scanner cmdScan;
    private Scanner dbScan;
    private Scanner lineScan;

    private File database;
    private File log;

    private FileWriter fw;
    private BufferedWriter bw;

    private String[] pool;
    private int poolsize = 0;

    HashTable nameTable = new HashTable(1024);

    prQuadTree<Point> coorTable;

    /**
     * Empty Constructor
     */
    public Parser() {

    }

    /**
     * Main parsing method
     * 
     * @param db
     *            database file name
     * @param cmd
     *            command script file name
     * @param log
     *            log file name
     * @throws IOException
     */
    public void Parsing(String db, String cmd, String lg) throws IOException {

        // scan the command file contents
        cmdScan = new Scanner(new File(cmd));
        log = new File(lg);
        fw = new FileWriter(log);
        bw = new BufferedWriter(fw);
        
        database = new File(db);
        FileWriter ffw = new FileWriter(database);
        BufferedWriter bbw = new BufferedWriter(ffw);

        pool = new String[15];

        String currentline;
        String command;
        String token;
        String filename = "";
        int count = 0;

        while (cmdScan.hasNextLine()) {

            currentline = cmdScan.nextLine();

            // skip the comment lines
            while (currentline.contains(";"))
                currentline = cmdScan.nextLine();

            if (count > 0)
                bw.write(String.format("Command %d:  %s\n\n", count,
                        currentline));
            else
                bw.write(String.format("%s\n\n", currentline));

            // scan current line to lineScan
            lineScan = new Scanner(currentline);
            command = lineScan.next();

            // implement according to the given command
            if (command.equals("debug")) {

                token = lineScan.next();

                if (token.equals("quad")) {
                    // debug quad

                    coorTable.printTree(coorTable.root, bw);
                    bw.write(String.format(
                            "--------------------------------------------------------------------------------\n"));
                }
                else if (token.equals("hash")) {
                    // debug hash

                    bw.write(String.format("Format of display is\n"));
                    bw.write(String.format("Slot number: data record\n"));
                    bw.write(String.format("Current table size is %d\n",
                            nameTable.size));
                    bw.write(String.format(
                            "Number of elements in table is %d\n\n",
                            nameTable.length));

                    for (int i = 0; i < nameTable.size; i++) {

                        if (nameTable.table[i] != null) {
                            bw.write(String.format("%8d: [%s:%s, %s]\n", i,
                                    nameTable.table[i].getFeature(),
                                    nameTable.table[i].getState(),
                                    nameTable.table[i].getOffset()));
                        }
                    }
                    bw.write(String.format(
                            "--------------------------------------------------------------------------------\n"));
                }
                else {
                    // debug pool

                    bw.write(String.format("MRU\n"));

                    for (int i = 0; i < poolsize; i++) {

                        bw.write(String.format("%s\n", pool[i]));
                    }

                    bw.write(String.format("LRU\n"));
                    bw.write(String.format(
                            "--------------------------------------------------------------------------------\n"));
                }
            }
            else if (command.equals("what_is")) {
                // what_is

                what_isHelper(db, currentline);
                bw.write(String.format(
                        "--------------------------------------------------------------------------------\n"));
            }
            else if (command.equals("what_is_at")) {
                // what_is_at

                String lat = lineScan.next();
                String lon = lineScan.next();
                Point checkpoint = new Point(DMS(lon), DMS(lat));

                if (coorTable.find(checkpoint) != null) {

                    what_is_atHelper(db, lat, lon);
                    bw.write(String.format(
                            "--------------------------------------------------------------------------------\n"));
                }
            }
            else if (command.equals("what_is_in")) {
                // what_is_in

                what_is_inHelper(filename, currentline);
                bw.write(String.format(
                        "--------------------------------------------------------------------------------\n"));
            }
            else if (command.equals("world")) {

                Date date = new Date();

                String xlo = lineScan.next();
                String xhi = lineScan.next();
                String ylo = lineScan.next();
                String yhi = lineScan.next();
                coorTable = new prQuadTree<Point>(DMS(xlo), DMS(xhi), DMS(ylo),
                        DMS(yhi));

                bw.write(String.format("GIS program\n\n"));
                bw.write(String.format("dbFile:\t%s\n", db));
                bw.write(String.format("script:\t%s\n", cmd));
                bw.write(String.format("log:\t%s\n", lg));
                bw.write(String.format("Start time: %s\n", date.toString()));
                bw.write(String.format(
                        "Quadtree children are printed in the order SW  SE  NE  NW\n"));
                bw.write(String.format(
                        "--------------------------------------------------------------------------------\n"));
                bw.write(String.format(
                        "Latitude/longitude values in index entries are shown as signed integers, in total seconds.\n\n"));
                bw.write(String.format("World boundaries are set to:\n"));
                bw.write(String.format("%30d\n", DMS(yhi)));
                bw.write(String.format("%20d%20d\n", DMS(xlo), DMS(xhi)));
                bw.write(String.format("%30d\n", DMS(ylo)));

                bw.write(String.format(
                        "--------------------------------------------------------------------------------\n"));

            }
            else if (command.equals("import")) {

                ffw = new FileWriter(database);
                bbw = new BufferedWriter(ffw);

                filename = lineScan.next();

                dbScan = new Scanner(new File(filename));
                iterateDBFile(dbScan, bbw);
                
                if (bbw != null)
                    bbw.close();

                if (ffw != null)
                    ffw.close();

                bw.write(String.format("Imported Features by name:\t%d\n",
                        nameTable.number));
                bw.write(String.format("Longest prob sequence:\t%d\n",
                        nameTable.worst));
                bw.write(String.format("Imported Location:\t\t%d\n",
                        coorTable.length));
                bw.write(String.format(
                        "--------------------------------------------------------------------------------\n"));
            }
            else {
                // quit
                Date date = new Date();

                bw.write(String.format("Terminating execution of commands.\n"));
                bw.write(String.format("End time: %s\n", date.toString()));
                bw.write(String.format(
                        "--------------------------------------------------------------------------------\n"));
                break;
            }
            count++;
        }// End while

        if (bw != null)
            bw.close();
        if (fw != null)
            fw.close();
    }

    /**
     * database file iterator
     * 
     * @param dbscan
     *            database scanner
     * @throws IOException
     */
    public void iterateDBFile(Scanner dbscan, BufferedWriter writer)
            throws IOException {

        String currentLine = dbscan.nextLine();
        String[] contents;

        long offset = currentLine.length() + 1;
        writer.write(String.format("%s\n", currentLine));

        while (dbscan.hasNextLine()) {

            currentLine = dbscan.nextLine();

            writer.write(String.format("%s\n", currentLine));
            contents = currentLine.split("\\|", 19);
            
            if((contents[8].equals("Unknown") || contents[7].equals("Unknown"))) {
                
            }
            else{
                
                Point newcoor = new Point(DMS(contents[8]), DMS(contents[7]));
                newcoor.setOff(offset);

                coorTable.insert(newcoor);
            }
            nameTable.insert(contents[1], contents[3], (int) offset);
            
            // }

            offset += currentLine.length() + 1;
        }
    }

    /**
     * what_is helper method
     * 
     * @param db
     *            database file name
     * @throws IOException
     */
    public void what_isHelper(String db, String currentline)
            throws IOException {

        String[] command = currentline.split("\t");
        String name = command[1];
        String state = command[2];
        int offset = 0;

        Scanner dbscan = new Scanner(new File(db));
        String currentL;
        String[] contents;
        String lat;
        String lon;

        while (dbscan.hasNextLine()) {

            currentL = dbscan.nextLine();
            contents = currentL.split("\\|", 19);

            if (contents[1].equals(name) && contents[3].equals(state)) {

                addPool(currentL, offset);
                
                lon = LonAndLatConv(contents[7]);
                lat = LonAndLatConv(contents[8]);

                bw.write(String.format("%8d:  %s  (%s, %s)\n", offset,
                        contents[5], lat, lon));
            }

            offset += currentL.length() + 1;
        }
    }

    /**
     * @param db
     * @throws IOException
     */
    public void what_is_atHelper(String db, String lat, String lon)
            throws IOException {

        Scanner dbscan = new Scanner(new File(db));
        String currentL;
        String[] contents;
        int offset = 0;

        bw.write(String.format(
                "   The following features were fount at (%s, %s): \n",
                LonAndLatConv(lon), LonAndLatConv(lat)));

        while (dbscan.hasNextLine()) {

            currentL = dbscan.nextLine();

            if (currentL.contains(lon) && currentL.contains(lat)) {

                addPool(currentL, offset);

                contents = currentL.split("\\|", 19);
                bw.write(String.format("%8d:  %s  %s  %s\n", offset,
                        contents[1], contents[5], contents[3]));
            }

            offset += currentL.length() + 1;
        }
    }

    /**
     * @param db
     * @param currentline
     * @throws IOException
     */
    public void what_is_inHelper(String db, String currentline)
            throws IOException {

        String[] contents = currentline.split("\t");

        if (contents.length == 5) {

            long xLo = DMS(contents[2]) - Long.parseLong(contents[4]);
            long xHi = DMS(contents[2]) + Long.parseLong(contents[4]);
            long yLo = DMS(contents[1]) - Long.parseLong(contents[3]);
            long yHi = DMS(contents[1]) + Long.parseLong(contents[3]);

            ArrayList<Point> list = coorTable.find(xLo, xHi, yLo, yHi);

            bw.write(String.format(
                    "The following %d features were found in (%s +/- %s, %s +/- %s)\n",
                    list.size(), LonAndLatConv(contents[2]), contents[4],
                    LonAndLatConv(contents[1]), contents[3]));

            String currentl;
            int offset;

            for (int i = 0; i < list.size(); i++) {

                offset = 0;

                Scanner dbscan = new Scanner(new File(db));

                while (dbscan.hasNextLine()) {

                    currentl = dbscan.nextLine();

                    if (offset == list.get(i).getOff()) {

                        String[] dbcontents = currentl.split("\\|");

                        addPool(currentl, offset);

                        bw.write(String.format("%8d: %s  %s  (%s, %s)\n",
                                offset, dbcontents[1], dbcontents[3],
                                LonAndLatConv(dbcontents[8]),
                                LonAndLatConv(dbcontents[7])));
                        break;
                    }

                    offset += currentl.length() + 1;
                }

            }
        }
        else if (contents.length == 6) {
            // -long
            long xLo = DMS(contents[3]) - Long.parseLong(contents[5]);
            long xHi = DMS(contents[3]) + Long.parseLong(contents[5]);
            long yLo = DMS(contents[2]) - Long.parseLong(contents[4]);
            long yHi = DMS(contents[2]) + Long.parseLong(contents[4]);

            ArrayList<Point> list = coorTable.find(xLo, xHi, yLo, yHi);

            bw.write(String.format(
                    "The following %d features were found in (%s +/- %s, %s +/- %s)\n",
                    list.size(), LonAndLatConv(contents[3]), contents[5],
                    LonAndLatConv(contents[2]), contents[4]));

            String currentl;
            int offset;

            for (int i = 0; i < list.size(); i++) {

                offset = 0;

                Scanner dbscan = new Scanner(new File(db));

                while (dbscan.hasNextLine()) {

                    currentl = dbscan.nextLine();

                    if (offset == list.get(i).getOff()) {

                        String[] dbcontents = currentl.split("\\|");

                        addPool(currentl, offset);

                        bw.write(String.format("  Feature ID\t: %s\n",
                                dbcontents[0]));
                        bw.write(String.format("  Feature Name\t: %s\n",
                                dbcontents[1]));
                        bw.write(String.format("  Feature Cat\t: %s\n",
                                dbcontents[2]));
                        bw.write(String.format("  State\t\t: %s\n",
                                dbcontents[3]));
                        bw.write(String.format("  County\t: %s\n",
                                dbcontents[5]));
                        bw.write(String.format("  Longitude\t: %s\n",
                                LonAndLatConv(dbcontents[8])));
                        bw.write(String.format("  Latitude\t: %s\n",
                                LonAndLatConv(dbcontents[7])));
                        bw.write(String.format("  Elev in ft\t: %s\n",
                                dbcontents[16]));
                        bw.write(String.format("  USGS Quad\t: %s\n",
                                dbcontents[17]));
                        bw.write(String.format("  Date created\t: %s\n\n",
                                dbcontents[18]));

                        break;
                    }

                    offset += currentl.length() + 1;
                }

            }

        }
        else {
            // -filter [ pop | water | structure ]

            long xLo = DMS(contents[4]) - Long.parseLong(contents[6]);
            long xHi = DMS(contents[4]) + Long.parseLong(contents[6]);
            long yLo = DMS(contents[3]) - Long.parseLong(contents[5]);
            long yHi = DMS(contents[3]) + Long.parseLong(contents[5]);

            ArrayList<Point> list = coorTable.find(xLo, xHi, yLo, yHi);

            bw.write(String.format(
                    "The following features matching your criteria were found in (%s +/- %s, %s +/- %s)\n\n",
                    LonAndLatConv(contents[4]), contents[6],
                    LonAndLatConv(contents[3]), contents[5]));

            String currentl;
            int offset;
            
            int count = 0;

            for (int i = 0; i < list.size(); i++) {

                offset = 0;

                Scanner dbscan = new Scanner(new File(db));

                while (dbscan.hasNextLine()) {

                    currentl = dbscan.nextLine();
                    String[] dbcontents = currentl.split("\\|");
                    
                    if(currentl.contains("DMS")) {
                        
                        offset += currentl.length() + 1;
                        currentl = dbscan.nextLine();
                        dbcontents = currentl.split("\\|");
                    }
                        
                    Point compare = new Point(DMS(dbcontents[8]), DMS(dbcontents[7]));
                    
                    if (compare.equals(list.get(i))) {

                        if (contents[2].equals("water")) {

                            if (dbcontents[2].equals("Arroyo")
                                    || dbcontents[2].equals("Bay")
                                    || dbcontents[2].equals("Bend")
                                    || dbcontents[2].equals("Canal")
                                    || dbcontents[2].equals("Channel")
                                    || dbcontents[2].equals("Falls")
                                    || dbcontents[2].equals("Glacier")
                                    || dbcontents[2].equals("Gut")
                                    || dbcontents[2].equals("Harbor")
                                    || dbcontents[2].equals("Lake")
                                    || dbcontents[2].equals("Rapids")
                                    || dbcontents[2].equals("Reservoir")
                                    || dbcontents[2].equals("Sea")
                                    || dbcontents[2].equals("Spring")
                                    || dbcontents[2].equals("Stream")
                                    || dbcontents[2].equals("Swamp")
                                    || dbcontents[2].equals("Well")) {

                                addPool(currentl, offset);

                                //bw.write(String.format("%s\n", currentl));

                                bw.write(String.format(
                                        "%8d: %s  %s  (%s, %s)\n", offset,
                                        dbcontents[1], dbcontents[3],
                                        LonAndLatConv(dbcontents[8]),
                                        LonAndLatConv(dbcontents[7])));
                                count++;
                            }
                        }
                        else if (contents[2].equals("structure")) {

                            if (dbcontents[2].equals("Airport")
                                    || dbcontents[2].equals("Bridge")
                                    || dbcontents[2].equals("Building")
                                    || dbcontents[2].equals("Church")
                                    || dbcontents[2].equals("Dam")
                                    || dbcontents[2].equals("Hospital")
                                    || dbcontents[2].equals("Levee")
                                    || dbcontents[2].equals("Park")
                                    || dbcontents[2].equals("Post Office")
                                    || dbcontents[2].equals("School")
                                    || dbcontents[2].equals("Tower")
                                    || dbcontents[2].equals("Tunnel")) {

                                addPool(currentl, offset);
                                
                                //bw.write(String.format("%s\n", currentl));

                                bw.write(String.format(
                                        "%8d: %s  %s  (%s, %s)\n", offset,
                                        dbcontents[1], dbcontents[3],
                                        LonAndLatConv(dbcontents[8]),
                                        LonAndLatConv(dbcontents[7])));
                                
                                count++;
                            }
                        }
                        else {

                            addPool(currentl, offset);

                            bw.write(String.format("%8d: %s  %s  (%s, %s)\n",
                                    offset, dbcontents[1], dbcontents[3],
                                    LonAndLatConv(dbcontents[8]),
                                    LonAndLatConv(dbcontents[7])));
                            
                            count++;
                        }

                    }

                    offset += currentl.length() + 1;
                }
            }
            bw.write(String.format("\nThere were %d features of type %s\n", count, contents[2]));
        }
    }

    /**
     * add pool method
     * 
     * @param line
     *            line you are going to add
     * @param offset
     *            offset of the line
     */
    public void addPool(String line, int offset) {

        boolean check = true;
        int ind = 0;
        for (int i = 0; i < poolsize; i++) {

            if (pool[i].contains(line)) {

                ind = i;
                check = false;
            }
        }

        if (check) {

            if (poolsize == 15) {

                for (int i = poolsize; i > 1; i--) {

                    pool[i - 1] = pool[i - 2];
                    poolsize--;
                }
            }
            else {

                for (int i = poolsize; i > 0; i--) {

                    pool[i] = pool[i - 1];
                }
            }
        }
        else {

            if (ind == 0)
                poolsize--;
            else {
                for (int i = ind; i > 0; i--) {

                    pool[i] = pool[i - 1];
                    poolsize--;
                }
            }
        }
        pool[0] = String.format("%8d:  %s", offset, line);
        poolsize++;
    }

    /**
     * @param dms
     * @return
     */
    public String LonAndLatConv(String dms) {

        String result;

        if (dms.length() == 8) {

            if (dms.charAt(7) == 'W')
                result = String.format("%dd %dm %ds West",
                        Integer.parseInt(dms.substring(0, 3)),
                        Integer.parseInt(dms.substring(3, 5)),
                        Integer.parseInt(dms.substring(5, 7)));
            else
                result = String.format("%dd %dm %ds East",
                        Integer.parseInt(dms.substring(0, 3)),
                        Integer.parseInt(dms.substring(3, 5)),
                        Integer.parseInt(dms.substring(5, 7)));
        }
        else {

            if (dms.charAt(6) == 'N')
                result = String.format("%dd %dm %ds North",
                        Integer.parseInt(dms.substring(0, 2)),
                        Integer.parseInt(dms.substring(2, 4)),
                        Integer.parseInt(dms.substring(4, 6)));
            else
                result = String.format("%dd %dm %ds South",
                        Integer.parseInt(dms.substring(0, 2)),
                        Integer.parseInt(dms.substring(2, 4)),
                        Integer.parseInt(dms.substring(4, 6)));
        }

        return result;
    }

    /**
     * DMS primary latitude or longitude converter
     * 
     * @param DMS
     *            primary latitude or longitude
     * @return converted primary latitude pr longitude
     */
    public long DMS(String dms) {

        long result;

        if (dms.length() == 8) {

            result = Long.parseLong(dms.substring(0, 3)) * 3600
                    + Long.parseLong(dms.substring(3, 5)) * 60
                    + Long.parseLong(dms.substring(5, 7));

            if (dms.charAt(7) == 'W')
                result = result * -1;
        }
        else {
            result = Long.parseLong(dms.substring(0, 2)) * 3600
                    + Long.parseLong(dms.substring(2, 4)) * 60
                    + Long.parseLong(dms.substring(4, 6));

            if (dms.charAt(6) == 'S')
                result = result * -1;
        }
        return result;
    }
}

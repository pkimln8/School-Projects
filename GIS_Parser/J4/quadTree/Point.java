package quadTree;

import java.util.ArrayList;

import quadTree.Compare2D;
import quadTree.Direction;

public class Point implements Compare2D<Point> {

    private long xcoord;
    private long ycoord;
    private ArrayList<Long> offsetList;

    public Point() {
        xcoord = 0;
        ycoord = 0;
        offsetList = null;
    }

    public Point(long x, long y) {
        xcoord = x;
        ycoord = y;
    }

    public long getX() {
        return xcoord;
    }

    public long getY() {
        return ycoord;
    }
    
    public void setOff(Long offset) {
        
        offsetList = new ArrayList<Long>();
        offsetList.add(offset);
    }
    
    public long getOff() {
        return offsetList.get(0);
    }

    
    public void addOffset(long offset) {
        offsetList.add(offset);
    }

    public Direction directionFrom(long X, long Y) {
        
        if(xcoord > X && ycoord >= Y || xcoord == X && ycoord == Y)
            return Direction.NE;
        else if(xcoord <= X && ycoord > Y)
            return Direction.NW;
        else if(xcoord < X && ycoord <= Y)
            return Direction.SW;
        else if(xcoord >= X && ycoord < Y)
            return Direction.SE;
        else
            return Direction.NOQUADRANT;
    }

    public Direction inQuadrant(double xLo, double xHi, double yLo,
            double yHi) {
        
        double xMid = xLo + (xLo - xHi) / 2;
        double yMid = yLo + (yLo - yHi) / 2;
        
        if(inBox(xLo, xHi, yLo, yHi)) {
            
            if(xcoord > xMid && ycoord >= yMid)
                return Direction.NE;
            else if(xcoord <= xMid && ycoord > yMid)
                return Direction.NW;
            else if(xcoord < xMid && ycoord <= yMid)
                return Direction.SW;
            else if(xcoord >= xMid && yMid > ycoord)
                return Direction.SE;
            else
                return Direction.NE;
        }
        else
            return Direction.NOQUADRANT;
    }

    public boolean inBox(double xLo, double xHi, double yLo, double yHi) {

        if(xcoord >= xLo && xcoord <= xHi && ycoord >= yLo && ycoord <= yHi)
            return true;
        else 
            return false;
    }

    public String toString() {

        String string = "(" + xcoord + ", " + ycoord + ")";
        
        return string;
    }

    public boolean equals(Object o) {
        
        Point other = (Point) o;
        
        if(other.getX() == this.xcoord && other.getY() == this.ycoord)
            return true;
        
        return false;
    }
    // Additional methods as needed...
}
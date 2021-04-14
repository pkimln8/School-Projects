package quadTree;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.ArrayList;

public class prQuadTree<T extends Compare2D<? super T>> {

    // -------------------------------------------------------------------------
    abstract class prQuadNode {

        public Object NW;
    }

    class prQuadLeaf extends prQuadNode {

        ArrayList<T> Elements;
        static final int bucketsize = 4;

        public prQuadLeaf() {

            Elements = null;
        }

        public prQuadLeaf(T elem) {

            Elements = new ArrayList<T>(bucketsize);
            Elements.add(elem);
        }

        public long getoffs(T elem) {

            return elem.getOff();
        }
    }

    class prQuadInternal extends prQuadNode {

        prQuadNode NW, NE, SE, SW;

        public prQuadInternal() {

            // NW = null;
            // NE = null;
            // SE = null;
            // SW = null;
        }

    }

    // -------------------------------------------------------------------------
    public prQuadNode root;
    public int length;

    long xMin, xMax, yMin, yMax;

    // Initialize quadtree to empty state, representing the specified region.
    // Pre: xMin < xMax and yMin < yMax
    public prQuadTree(long xMin, long xMax, long yMin, long yMax) {

        root = null;

        this.xMin = xMin;
        this.xMax = xMax;
        this.yMin = yMin;
        this.yMax = yMax;
        length = 0;
    }

    // Pre: elem != null
    // Returns reference to an element x within the tree such that
    // elem.equals(x)is true, provided such a matching element occurs within
    // the tree; returns null otherwise.
    public T find(T Elem) {

        if (root == null)
            return null;
        else if (!Elem.inBox(xMin, xMax, yMin, yMax))
            return null;
        else
            return findHelp(Elem, root, xMin, xMax, yMin, yMax);
    }

    @SuppressWarnings("unchecked")
    private T findHelp(T elem, prQuadNode sRoot, long xLo, long xHi, long yLo,
            long yHi) {

        if (sRoot == null)
            return null;

        if (sRoot.getClass().equals(prQuadLeaf.class)) {

            prQuadLeaf leaf = (prQuadLeaf) sRoot;

            for (int i = 0; i < leaf.Elements.size(); i++) {

                if (leaf.Elements.get(i).equals(elem))
                    return leaf.Elements.get(i);
            }

            return null;
        }
        else {

            prQuadInternal internal = (prQuadInternal) sRoot;

            long xMid = xLo + (xHi - xLo) / 2;
            long yMid = yLo + (yHi - yLo) / 2;

            Direction dir = elem.directionFrom(xMid, yMid);

            if (dir == Direction.NE)
                return findHelp(elem, internal.NE, xMid, xHi, yMid, yHi);
            else if (dir == Direction.NW)
                return findHelp(elem, internal.NW, xLo, xMid, yMid, yHi);
            else if (dir == Direction.SW)
                return findHelp(elem, internal.SW, xLo, xMid, yLo, yMid);
            else
                return findHelp(elem, internal.SE, xMid, xHi, yLo, yMid);
        }
    }

    // Pre: elem != null
    // Post: If elem lies within the tree's region, and elem is not already
    // present in the tree, elem has been inserted into the tree.
    // Return true iff elem is inserted into the tree.
    public boolean insert(T elem) {

        if (elem == null || !elem.inBox(xMin, xMax, yMin, yMax))
            return false;
        else if (find(elem) != null) {

            find(elem).addOffset(elem.getOff());
            length++;
            return true;
        }
        else {

            root = insertHelp(elem, root, xMin, xMax, yMin, yMax);
            length++;
            return true;
        }
    }

    @SuppressWarnings("unchecked")
    private prQuadNode insertHelp(T elem, prQuadNode sRoot, long xLo, long xHi,
            long yLo, long yHi) {

        if (sRoot == null)
            return new prQuadLeaf(elem);

        if (sRoot.getClass().equals(prQuadLeaf.class)) {

            prQuadLeaf leaf = (prQuadLeaf) sRoot;

            if (leaf.Elements.size() < 4) {

                leaf.Elements.add(elem);

                return leaf;
            }
            else {
                prQuadNode inner = new prQuadInternal();

                //
                inner = insertHelp(leaf.Elements.get(0), inner, xLo, xHi, yLo,
                        yHi);
                inner = insertHelp(leaf.Elements.get(1), inner, xLo, xHi, yLo,
                        yHi);
                inner = insertHelp(leaf.Elements.get(2), inner, xLo, xHi, yLo,
                        yHi);
                inner = insertHelp(leaf.Elements.get(3), inner, xLo, xHi, yLo,
                        yHi);
                //
                inner = insertHelp(elem, inner, xLo, xHi, yLo, yHi);

                return inner;
            }
        }
        else {

            prQuadInternal internal = (prQuadInternal) sRoot;

            long xMid = xLo + ((xHi - xLo) / 2);
            long yMid = yLo + ((yHi - yLo) / 2);

            Direction dir = elem.directionFrom(xMid, yMid);

            if (dir == Direction.SE)
                internal.SE = insertHelp(elem, internal.SE, xMid, xHi, yLo,
                        yMid);
            else if (dir == Direction.NW)
                internal.NW = insertHelp(elem, internal.NW, xLo, xMid, yMid,
                        yHi);
            else if (dir == Direction.SW)
                internal.SW = insertHelp(elem, internal.SW, xLo, xMid, yLo,
                        yMid);
            else
                internal.NE = insertHelp(elem, internal.NE, xMid, xHi, yMid,
                        yHi);

            return internal;
        }
    }

    // Pre: elem != null
    // Post: If elem lies in the tree's region, and a matching element occurs
    // in the tree, then that element has been removed.
    // Returns true iff a matching element has been removed from the tree.
    public boolean delete(T Elem) {

        if (Elem == null)
            return false;
        else if (!Elem.inBox(xMin, xMax, yMin, yMax))
            return false;
        else if (find(Elem) == null)
            return false;
        else {

            root = deleteHelp(Elem, root, xMin, xMax, yMin, yMax);
            return true;
        }
    }

    @SuppressWarnings("unchecked")
    private prQuadNode deleteHelp(T elem, prQuadNode sRoot, long xLo, long xHi,
            long yLo, long yHi) {

        if (sRoot == null)
            return null;

        if (sRoot.getClass().equals(prQuadLeaf.class)) {

            return null;
        }

        else {

            prQuadInternal internal = (prQuadInternal) sRoot;

            long xMid = xLo + (xHi - xLo) / 2;
            long yMid = yLo + (yHi - yLo) / 2;

            Direction dir = elem.directionFrom(xMid, yMid);

            if (dir == Direction.NE)
                internal.NE = deleteHelp(elem, internal.NE, xMid, xHi, yMid,
                        yHi);
            else if (dir == Direction.NW)
                internal.NW = deleteHelp(elem, internal.NW, xLo, xMid, yMid,
                        yHi);
            else if (dir == Direction.SW)
                internal.SW = deleteHelp(elem, internal.SW, xLo, xMid, yLo,
                        yMid);
            else
                internal.SE = deleteHelp(elem, internal.SE, xMid, xHi, yLo,
                        yMid);

            int counter = 0;

            if (internal.NE != null) {

                if (!internal.NE.getClass().equals(prQuadInternal.class))
                    counter++;
            }

            if (internal.NW != null) {

                if (!internal.NW.getClass().equals(prQuadInternal.class))
                    counter++;
            }

            if (internal.SE != null) {

                if (!internal.SE.getClass().equals(prQuadInternal.class))
                    counter++;
            }
            if (internal.SW != null) {

                if (!internal.SW.getClass().equals(prQuadInternal.class))
                    counter++;
            }

            if (counter < 2) {

                if (internal.NE != null)
                    return internal.NE;
                else if (internal.NW != null)
                    return internal.NW;
                else if (internal.SE != null)
                    return internal.SE;
                else
                    return internal.SW;
            }

            return internal;
        }
    }

    // Pre: xLo < xHi and yLo < yHi
    // Returns a collection of (references to) all elements x such that x is
    // in the tree and x lies at coordinates within the defined rectangular
    // region, including the boundary of the region.
    public ArrayList<T> find(long xLo, long xHi, long yLo, long yHi) {

        ArrayList<T> elems = new ArrayList<T>();

        if (xLo >= xMax || xHi <= xMin || yLo >= yMax || yHi <= yMin)
            return elems;
        else
            return findHelp(elems, root, xLo, xHi, yLo, yHi, xMin, xMax, yMin,
                    yMax);
    }

    @SuppressWarnings("unchecked")
    private ArrayList<T> findHelp(ArrayList<T> elems, prQuadNode sRoot,
            long xLo, long xHi, long yLo, long yHi, long xMin, long xMax,
            long yMin, long yMax) {

        if (sRoot == null)
            return elems;

        if (sRoot.getClass().equals(prQuadLeaf.class)) {

            prQuadLeaf leaf = (prQuadLeaf) sRoot;

            for (int i = 0; i < leaf.Elements.size(); i++) {

                long X = leaf.Elements.get(i).getX();
                long Y = leaf.Elements.get(i).getY();

                if (xLo <= X && X <= xHi && yLo <= Y && Y <= yHi) {

                    elems.add(leaf.Elements.get(i));
                }
            }
        }
        else {

            prQuadInternal internal = (prQuadInternal) sRoot;

            long xMid = xMin + (xMax - xMin) / 2;
            long yMid = yMin + (yMax - yMin) / 2;

            if (xMid <= xHi && yMid <= yHi)
                findHelp(elems, internal.NE, xLo, xHi, yLo, yHi, xMid, xMax,
                        yMid, yMax);
            if (xMid >= xLo && yMid <= yHi)
                findHelp(elems, internal.NW, xLo, xHi, yLo, yHi, xMin, xMid,
                        yMid, yMax);
            if (xMid >= xLo && yMid >= yLo)
                findHelp(elems, internal.SW, xLo, xHi, yLo, yHi, xMin, xMid,
                        yMin, yMid);
            if (xMid <= xHi && yMid >= yLo)
                findHelp(elems, internal.SE, xLo, xHi, yLo, yHi, xMid, xMax,
                        yMin, yMid);
        }

        return elems;
    }

    /**
     * Prints all elements in the tree. Used for testing
     * 
     * @param root
     *            prQuadNode to begin traversing
     * @throws IOException
     */
    public void printTree(prQuadNode root, BufferedWriter bw)
            throws IOException {
        printTreeHelper(root, "", bw);
    }

    /**
     * helper method for printTree;
     * 
     * @param sRoot
     *            prQuadNode current node
     * @param Padding
     * @throws IOException
     */
    @SuppressWarnings("unchecked")
    public void printTreeHelper(prQuadNode sRoot, String Padding,
            BufferedWriter bw) throws IOException {
        // Check for empty leaf
        if (sRoot == null) {
            return;
        }
        // Check for and process SW and SE subtrees
        if (!sRoot.getClass().equals(prQuadLeaf.class)) {
            prQuadInternal p = (prQuadInternal) sRoot;

            bw.write(String.format("%s*\n", Padding));
            printTreeHelper(p.SW, Padding + "\t", bw);
            printTreeHelper(p.SE, Padding + "\t", bw);
            // bw.write(String.format("%s*\n", Padding));
        }
        // Display indentation padding for current node

        // Determine if at leaf or internal and display accordingly
        if (sRoot.getClass().equals(prQuadLeaf.class)) {
            prQuadLeaf p = (prQuadLeaf) sRoot;
            bw.write(String.format("%s", Padding));

            for (int i = 0; i < p.Elements.size(); i++) {

                bw.write(String.format("[%s, %s] ", p.Elements.get(i),
                        p.Elements.get(i).getOff()));
            }
            bw.write(String.format("\n"));
        }
        else {

            bw.write(String.format("%s @\n", Padding));
        }

        if (!sRoot.getClass().equals(prQuadLeaf.class)) {
            prQuadInternal p = (prQuadInternal) sRoot;

            // bw.write(String.format("%s*\n", Padding));
            printTreeHelper(p.NE, Padding + "\t", bw);
            printTreeHelper(p.NW, Padding + "\t", bw);
        }

    }
}

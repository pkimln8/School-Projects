package hash;

import java.util.ArrayList;

public class Entry implements Element3<Entry> {

    private String feature;
    private String state;
    private ArrayList<Long> offsets;

    public Entry() {

        feature = null;
        state = null;
    }

    public Entry(String f, String s, long offset) {

        feature = f;
        state = s;
        offsets = new ArrayList<Long>();
        offsets.add(offset);
    }

    @Override
    public String getFeature() {

        return feature;
    }

    @Override
    public String getState() {

        return state;
    }
    
    @Override
    public ArrayList<Long> getOffset() {
        
        return offsets;
    }
    
    public void addOffset(long offset) {
        
        offsets.add(offset);
    }
    
    @Override
    public long elfHash(String toHash) {

        long hashValue = 0;
        for (int Pos = 0; Pos < toHash.length(); Pos++) { // use all elements

            hashValue = (hashValue << 4) + toHash.charAt(Pos);  // shift/mix

            long hiBits = hashValue & 0xF000000000000000L;      // get high
                                                                // nybble

            if (hiBits != 0) {
                hashValue ^= hiBits >> 56; // xor high nybble with second nybble
            }

            hashValue &= ~hiBits;         // clear high nybble
        }

        return (hashValue & 0x0FFFFFFFFFFFFFFFL);
    }


}

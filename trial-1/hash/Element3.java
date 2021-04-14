package hash;

import java.util.ArrayList;

public interface Element3<T> {

    public String getFeature();
    
    public String getState();
    
    public ArrayList<Long> getOffset();

    public long elfHash(String toHash);
}

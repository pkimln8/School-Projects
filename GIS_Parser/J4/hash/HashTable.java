package hash;

public class HashTable {

    public int length = 0;
    public int worst = 0;
    public int size = 0;
    public int number = 0;

    public Entry[] table;

    public HashTable(int size) {

        this.size = size;
        table = new Entry[size];
    }

    public void insert(String feature, String state, int offset) {

        Entry newEntry = new Entry(feature, state, offset);

        boolean found = false;

        for (int i = 0; i < size; i++) {

            if (table[i] != null) {

                if (table[i].getFeature().equals(newEntry.getFeature())
                        && table[i].getState().equals(newEntry.getState())) {

                    table[i].addOffset(newEntry.getOffset().get(0));
                    number++;
                    found = true;
                }

            }
        }

        if (!found) {
            int index = (int) (Math.abs(newEntry.elfHash(newEntry.getFeature()))
                    % size);

            // rehashing: table is filled over 70%
            if (length > size * 0.7) {

                size = size * 2;
                Entry[] newtable = new Entry[size];

                for (int i = 0; i < size / 2; i++) {

                    if (table[i] != null) {

                        int newindex = (int) (Math
                                .abs(table[i].elfHash(table[i].getFeature()))
                                % size);

                        if (newtable[newindex] == null) {

                            newtable[newindex] = table[i];
                        }
                        else {

                            int j = 0;
                            while (newtable[newindex] != null) {

                                j++;

                                newindex = (newindex + ((j * j + j) / 2))
                                        % size;

                                if (j > worst)
                                    worst = j;
                            }

                            newtable[newindex] = table[i];
                        }
                    }
                }

                table = newtable;
            }

            // Inserting
            if (table[index] == null) {

                table[index] = newEntry;
                length++;
                number++;
            }
            else {
                // when the index is duplicating

                int i = 0;
                while (table[index] != null) {
                    // probing
                    i++;

                    index = (index + ((i * i + i) / 2)) % size;

                    if (i > worst)
                        worst = i;
                }

                table[index] = newEntry;
                length++;
                number++;
            }
        }

    }

    public Entry[] reHashing(Entry[] oldHashTable) {

        size = size * 2;

        Entry[] newTable = new Entry[size];

        for (int i = 0; i < size / 2; i++) {

            if (oldHashTable[i] != null) {
            }

        }

        return newTable;
    }
}

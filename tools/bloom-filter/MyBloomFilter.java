import java.util.BitSet;

public class MyBloomFilter {
    private BitSet bitSet;
    private int size;
    private int hashCount;

    public MyBloomFilter(int size, int hashCount) {
        this.size = size;
        this.hashCount = hashCount;
        this.bitSet = new BitSet(size);
    }

    private int[] getHashes(String item) {
        int[] hashes = new int[hashCount];
        for (int i = 0; i < hashCount; i++) {
            hashes[i] = Math.abs((item.hashCode() + i * 31) % size);
        }
        return hashes;
    }

    public void add(String item) {
        int[] hashes = getHashes(item);
        for (int hash : hashes) {
            bitSet.set(hash);
        }
    }

    public boolean contains(String item) {
        int[] hashes = getHashes(item);
        for (int hash : hashes) {
            if (!bitSet.get(hash)) {
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args) {
        MyBloomFilter bloomFilter = new MyBloomFilter(1000, 5);

        bloomFilter.add("hello");
        bloomFilter.add("world");

        System.out.println(bloomFilter.contains("hello")); // true
        System.out.println(bloomFilter.contains("world")); // true
        System.out.println(bloomFilter.contains("java"));  // false
    }
}
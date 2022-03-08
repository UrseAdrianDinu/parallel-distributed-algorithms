import java.util.ArrayList;
import java.util.Map;

public class MapResult<K,V> {
    ArrayList<String> longest_strings;
    Map<K, V> map;
    String filename;

    public MapResult(ArrayList<String> longeststrings,
                     Map<K, V> map,
                     String filename) {
        this.longest_strings = longeststrings;
        this.map = map;
        this.filename = filename;
    }


    @Override
    public String toString() {
        return "MapResult{" +
                "longeststrings=" + longest_strings +
                ", map=" + map +
                ", filename='" + filename + '\'' +
                '}';
    }
}

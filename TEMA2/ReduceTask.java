import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;

public class ReduceTask implements Runnable {

    private final String filename;
    private final CompletableFuture<ReduceResult> completableFuture;
    ArrayList<MapResult<Integer, Integer>> results;

    public ReduceTask(String filename, ArrayList<MapResult<Integer, Integer>> results, CompletableFuture<ReduceResult> completableFuture) {
        this.filename = filename;
        this.results = results;
        this.completableFuture = completableFuture;
    }

    // Metoda care intoarce al n lea element din sirul lui Fibonacci
    float fib(float n) {
        if (n <= 1) {
            return n;
        }
        return fib(n - 1) + fib(n - 2);
    }

    @Override
    public void run() {

        // Array ce contine cele mai lungi cuvinte pentru intregul document
        ArrayList<String> longest_strings = new ArrayList<>();
        longest_strings.add("");

        // Dictionar cu dimensiunile cuvintelor si numarul lor de aparitii pentru intregul document
        Map<Integer, Integer> map = new HashMap<Integer, Integer>();

        // Numarul de cuvinte din document
        float words_count = 0;


        // Etapa de combinare
        // Parcurgere vector de rezultate pentru documentul curent
        for (MapResult<Integer, Integer> result : results) {
            // Daca cel mai lung cuvant pentru rezultatul curent este mai lung decat cel mai lung cuvant
            // pentru documentul curent, atunci sterg toate elementele din longest_strings
            // si adaug toate cuvintele din rezultatul curent
            if (result.longest_strings.get(0).length() > longest_strings.get(0).length()) {
                longest_strings.clear();
                longest_strings.addAll(result.longest_strings);
            } else {
                // Daca au aceeasi lungime, adaug toate cuvintele in vectorul longest_strings
                if (result.longest_strings.get(0).length() == longest_strings.get(0).length())
                    longest_strings.addAll(result.longest_strings);
            }

            // Parcurg dictionarul rezultatului curent
            for (Map.Entry<Integer, Integer> entry : result.map.entrySet()) {
                Integer key = entry.getKey();
                Integer value = entry.getValue();

                // Adun la numarul de cuvinte, numarul de aparitii al cuvantului curent
                words_count += value;

                // Daca dictionarul contine o intrare ce are cheia egala cu
                // lungimea cuvantului curent, atunci adaug la valoarea curenta
                // numarul de aparitii al cuvantului curent,
                // In caz contrar, adaug o intrare noua

                if (!map.containsKey(key)) {
                    map.put(key, value);
                } else {
                    map.put(key, map.get(key) + value);
                }
            }
        }

        // Etapa de procesare
        // Parcurg dictionarul final si calculez rangul
        float sum = 0;
        for (Map.Entry<Integer, Integer> entry : map.entrySet()) {
            Integer key = entry.getKey();
            Integer value = entry.getValue();
            float fibo = fib(key + 1);
            sum += fibo * value;
        }

        float rang = sum / words_count;

        // Completez future-ul cu rezultatul calculat
        ReduceResult result = new ReduceResult(filename, rang, longest_strings.get(0).length(), longest_strings.size());
        completableFuture.complete(result);

    }
}

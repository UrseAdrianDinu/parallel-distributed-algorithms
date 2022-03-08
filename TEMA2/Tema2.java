import java.io.*;
import java.util.*;
import java.util.concurrent.*;


public class Tema2 {
    public static void main(String[] args) throws FileNotFoundException, ExecutionException, InterruptedException {

        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }

        // Citire argumente
        int workers = Integer.parseInt(args[0]);
        String in_file = args[1];
        String out_file = args[2];

        // Operatia Map
        MapOperation mapOperation = new MapOperation(workers, in_file);
        ArrayList<MapResult<Integer, Integer>> mapResults = mapOperation.map();

        // Operatia Reduce
        ReduceOperation reduceOperation = new ReduceOperation(workers, mapResults, mapOperation.getNo_files(), mapOperation.getInput_files());
        ArrayList<ReduceResult> reduceResults = reduceOperation.reduce(mapResults);

        // Sortare descrecatoare dupa rang
        // In cazul in care doua (sau mai multe) documente au acelasi rang, sunt sortate dupa
        // ordinea in care apar in fisierul de intrare

        reduceResults.sort(new Comparator<>() {

            @Override
            public int compare(ReduceResult o1, ReduceResult o2) {
                if (o2.rang > o1.rang) {
                    return 1;
                } else {
                    if (o2.rang < o1.rang) {
                        return -1;
                    }
                }
                ArrayList<String> files = mapOperation.getInput_files();
                return files.indexOf(o1.filename) - files.indexOf(o2.filename);
            }
        });

        // Scrierea rezultatelor in fisierul de iesire
        PrintWriter writer = new PrintWriter(out_file);
        for (ReduceResult reduceResult : reduceResults) {
            String filename = reduceResult.filename;
            float rang = reduceResult.rang;
            int longest_word = reduceResult.longestword;
            int occ = reduceResult.occurences;
            String[] s = filename.split("/");
            writer.println(s[2] + "," + String.format("%.2f", rang) + "," + longest_word + "," + occ);
        }
        writer.close();

    }
}

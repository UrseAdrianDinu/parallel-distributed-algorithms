import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.concurrent.*;

public class MapOperation implements MapInterface<Integer, Integer> {

    // Numarul de worker-i
    private final int workers;

    // Numele fisierului de intrare
    private final String in_file;

    // Numarul documentelor
    private int no_files;

    // Vector cu numele documentelor
    private ArrayList<String> input_files;

    public MapOperation(int workers, String in_file) {
        this.workers = workers;
        this.in_file = in_file;
    }

    public ArrayList<String> getInput_files() {
        return input_files;
    }

    public int getNo_files() {
        return no_files;
    }

    @Override
    public ArrayList<MapResult<Integer, Integer>> map() {

        // Pool pentru task-urile Map
        ExecutorService map_pool = Executors.newFixedThreadPool(workers);

        // Citire dimenisune fragment si numarul de fisiere
        int fragment_dim;

        Scanner scanner_in = null;
        try {
            scanner_in = new Scanner(new File(in_file));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        assert scanner_in != null;
        fragment_dim = scanner_in.nextInt();
        no_files = scanner_in.nextInt();

        // Array pentru rezultatele etapei Map
        ArrayList<MapResult<Integer, Integer>> mapResults = new ArrayList<>();

        // Array pentru numele fisierelor de input
        input_files = new ArrayList<>();

        int curr_dim;
        int curr_offset;

        ArrayList<CompletableFuture<MapResult<Integer, Integer>>> futures = new ArrayList<>();

        // Impartirea documentului pe fragmente si crearea taskurilor Map
        for (int i = 0; i < no_files; i++) {
            String filename = scanner_in.next();
            input_files.add(filename);
            File file = new File(filename);
            long length = file.length();
            curr_offset = 0;
            curr_dim = fragment_dim;
            while (curr_offset < length) {
                if (curr_offset + curr_dim > length) {
                    curr_dim = (int) (length - curr_offset);
                }
                CompletableFuture<MapResult<Integer, Integer>> completableFuture = new CompletableFuture<>();
                // Adaug task-urile Map in pool
                map_pool.submit(new MapTask(filename, curr_offset, curr_dim, completableFuture));
                futures.add(completableFuture);
                curr_offset += fragment_dim;
            }
        }

        // Preluarea rezultatelor din obiectele Future
        for (CompletableFuture<MapResult<Integer, Integer>> future : futures) {
            MapResult<Integer, Integer> result = null;
            try {
                result = future.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
            mapResults.add(result);
        }

        map_pool.shutdown();

        return mapResults;
    }
}

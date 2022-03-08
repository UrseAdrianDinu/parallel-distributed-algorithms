import java.io.*;
import java.util.*;
import java.util.concurrent.CompletableFuture;

public class MapTask implements Runnable {
    private final String filename;
    private final int offset;
    private final int frag_dim;
    private final CompletableFuture<MapResult<Integer,Integer>> completableFuture;

    public MapTask(String filename, int offset, int frag_dim,
                   CompletableFuture<MapResult<Integer,Integer>> completableFuture) {
        this.filename = filename;
        this.offset = offset;
        this.frag_dim = frag_dim;
        this.completableFuture = completableFuture;
    }

    @Override
    public void run() {

        try {
            RandomAccessFile raf = new RandomAccessFile(filename, "r");

            byte[] bytes = new byte[frag_dim];

            // Flag care indica daca caracterul dinaintea fragmentului curent este litera
            boolean prev_is_char = false;

            // Array pentru cuvintele de lungime maxima
            ArrayList<String> longest_strings = new ArrayList<>();
            longest_strings.add("");

            // Dictionar cu dimensiunile cuvintelor si numarul lor de aparitii
            Map<Integer, Integer> map = new HashMap<>();

            // Daca offset-ul nu este 0 citesc caracterul dinaintea fragmentului curent
            // si verific daca este litera
            if (offset != 0) {
                raf.seek(offset - 1);
                byte prevbyte = raf.readByte();
                char prevchar = (char) prevbyte;
                if (Character.isLetter(prevchar)) {
                    prev_is_char = true;
                }
            }

            // Citesc fragmentul curent
            raf.read(bytes);
            String buffer = new String(bytes);

            // Parsez buffer-ul in cuvinte
            String separators = ";:/?~\\.,><`[]{}()!@#$%^&-_+'=*\"| \t\r\n";
            StringTokenizer tokenizer = new StringTokenizer(buffer, separators);

            int count = tokenizer.countTokens();
            int k = 0;

            // Daca caracterul dinaintea fragmentului curent este si
            // primul caracter din fragmentul curent sunt litere, atunci
            // sar peste primul cuvant
            if (prev_is_char && Character.isLetter(buffer.charAt(0))) {
                tokenizer.nextToken();
                k++;
            }

            // Parcurg cuvintele pana la ultimul
            while (k < count - 1) {
                String str = tokenizer.nextToken();

                // Daca cuvantul curent are lungimea mai mare decat cel mai lung cuvant gasit
                // atunci sterg toate elementele din vectorul longest_strings
                // si adaug cuvantul gasit
                // Daca are aceeasi lungime, il adaug in array
                if (str.length() > longest_strings.get(0).length()) {
                    longest_strings.clear();
                    longest_strings.add(str);
                } else {
                    if (str.length() == longest_strings.get(0).length()) {
                        longest_strings.add(str);
                    }
                }

                // Daca dictionarul contine o intrare ce are cheia egala cu
                // lungimea cuvantului curent, incrementez valoarea
                // In caz contrar adaug o noua intrare
                if (!map.containsKey(str.length())) {
                    map.put(str.length(), 1);
                } else {
                    int val = map.get(str.length());
                    map.put(str.length(), val + 1);
                }
                k++;
            }

            // Extrag ultimul cuvant
            // Verific daca dupa el urmeaza un separator
            // Daca nu este separator, citesc caractere pana cand intalnesc un separator sau EOF
            // si le adaug la ultimul cuvant
            if (tokenizer.hasMoreTokens()) {
                StringBuilder last = new StringBuilder(tokenizer.nextToken());
                if (separators.indexOf(buffer.charAt(buffer.length() - 1)) == -1) {
                    try {
                        byte nextbyte = raf.readByte();
                        char nextchar = (char) nextbyte;
                        if (Character.isLetter(nextchar)) {
                            while (Character.isLetter(nextchar)) {
                                last.append(nextchar);
                                nextbyte = raf.readByte();
                                nextchar = (char) nextbyte;
                            }
                        }
                    } catch (EOFException e) {

                    }
                }

                // Daca ultimul cuvant are lungimea mai mare decat cel mai lung cuvant gasit
                // atunci sterg toate elementele din vectorul longest_strings
                // si il adaug in vector
                // Daca are aceeasi lungime, il adaug in array
                if (last.length() > longest_strings.get(0).length()) {
                    longest_strings.clear();
                    longest_strings.add(String.valueOf(last));
                } else {
                    if (last.length() == longest_strings.get(0).length()) {
                        longest_strings.add(String.valueOf(last));
                    }
                }

                // Daca dictionarul contine o intrare ce are cheia egala cu
                // lungimea ultimului cuvant, atunci incrementez valoarea
                // In caz contrar adaug o noua intrare
                if (!map.containsKey(last.length())) {
                    map.put(last.length(), 1);
                } else {
                    int val = map.get(last.length());
                    map.put(last.length(), val + 1);
                }
            }

            // Completez future-ul cu rezultatul calculat
            MapResult<Integer,Integer> mapResult = new MapResult<>(longest_strings, map, filename);
            completableFuture.complete(mapResult);


        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

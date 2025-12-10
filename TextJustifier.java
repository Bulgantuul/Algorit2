import java.util.List;
import java.util.Scanner;
import java.util.ArrayList;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;

public class TextJustifier {

    private static int calculateCost(int start, int end, String[] words, int maxWidth) {
        int wordLengthSum = 0;
        for (int i = start; i <= end; i++) {
            wordLengthSum += words[i].length();
        }
        int spaces = end - start;
        int totalLength = wordLengthSum + spaces;
        if (totalLength > maxWidth) {
            return Integer.MAX_VALUE;
        }
        int remainingSpaces = maxWidth - totalLength;
        if (end == words.length - 1) {
            return 0;
        }

        return remainingSpaces * remainingSpaces * remainingSpaces;
    }

    private static String formatLine(String[] words, int maxWidth, int start, int end, boolean isLastLine) {
        StringBuilder line = new StringBuilder();
        int numWords = end - start + 1;

        if (numWords == 1 || isLastLine) {
            for (int i = start; i <= end; i++) {
                line.append(words[i]);
                if (i < end)
                    line.append(" ");
            }
            while (line.length() < maxWidth)
                line.append(" ");
            return line.toString();
        }

        int totalWordLength = 0;
        for (int i = start; i <= end; i++)
            totalWordLength += words[i].length();

        int totalSpaces = maxWidth - totalWordLength;
        int numGaps = numWords - 1;

        int baseSpaces = totalSpaces / numGaps;
        int extraSpaces = totalSpaces % numGaps;

        for (int i = start; i <= end; i++) {
            line.append(words[i]);
            if (i < end) {
                int spacesToAdd = baseSpaces + (i - start < extraSpaces ? 1 : 0);
                line.append(" ".repeat(spacesToAdd));
            }
        }

        return line.toString();
    }

    public static List<String> justifyGreedy(String[] words, int maxWidth) {
        List<String> result = new ArrayList<>();
        int i = 0;
        int n = words.length;

        while (i < n) {
            int j = i;
            int currentLength = 0;

            while (j < n &&
                    (currentLength + words[j].length() + (j > i ? 1 : 0)) <= maxWidth) {

                currentLength += words[j].length() + (j > i ? 1 : 0);
                j++;
            }

            result.add(formatLine(words, maxWidth, i, j - 1, j == n));
            i = j;
        }
        return result;
    }

    public static List<String> justifyDP(String[] words, int maxWidth) {
        int n = words.length;
        int[] cost = new int[n + 1];
        int[] parent = new int[n + 1];

        cost[n] = 0;

        for (int i = n - 1; i >= 0; i--) {
            cost[i] = Integer.MAX_VALUE;

            for (int j = i + 1; j <= n; j++) {
                int currCost = calculateCost(i, j - 1, words, maxWidth);

                if (currCost == Integer.MAX_VALUE)
                    continue;
                if (cost[j] == Integer.MAX_VALUE)
                    continue;

                int totalCost = currCost + cost[j];
                if (totalCost < cost[i]) {
                    cost[i] = totalCost;
                    parent[i] = j;
                }
            }
        }

        List<String> result = new ArrayList<>();
        int i = 0;

        while (i < n) {
            int j = parent[i];
            result.add(formatLine(words, maxWidth, i, j - 1, j == n));
            i = j;
        }

        return result;
    }

    public static void main(String[] args) {

        try {
            System.setOut(new PrintStream(System.out, true, StandardCharsets.UTF_8.name()));
        } catch (UnsupportedEncodingException e) {
            System.err.println("UTF-8 encoding not supported!");
        }

        String text;
        int maxWidth = -1;

        try (Scanner scanner = new Scanner(System.in, StandardCharsets.UTF_8.name())) {
            System.out.print("Бичвэрээ оруулна уу: ");
            text = scanner.nextLine();

            while (maxWidth <= 0) {
                System.out.print("Мөрийн хамгийн их уртыг (бүхэл тоо) оруулна уу: ");
                if (scanner.hasNextInt()) {
                    maxWidth = scanner.nextInt();
                    scanner.nextLine();
                    if (maxWidth <= 0) {
                        System.err.println("Мөрийн урт 0-ээс их байх ёстой.");
                    }
                } else {
                    System.err.println("Буруу оролт. Тоо оруулна уу.");
                    scanner.next();
                }
            }

            String[] words = text.split("\\s+");
            if (words.length == 0 || (words.length == 1 && words[0].isEmpty())) {
                System.out.println("Оролтод үг байхгүй байна.");
                return;
            }

            System.out.println("\n--- Оролт Баталгаажуулалт ---");
            System.out.println("Мөрийн хамгийн их урт (M): " + maxWidth);
            System.out.println("Нийт үг (N): " + words.length);
            System.out.println("--------------------------------\n");

            long t1 = System.nanoTime();
            List<String> greedyResult = justifyGreedy(words, maxWidth);
            long t2 = System.nanoTime();

            System.out.println("1. Greedy Algorithm");
            greedyResult.forEach(System.out::println);
            long greedyTime = t2 - t1;
            System.out.printf("Гүйцэтгэл: %,d ns%n", greedyTime);
            System.out.println("--------------------------------\n");

            long t3 = System.nanoTime();
            List<String> dpResult = justifyDP(words, maxWidth);
            long t4 = System.nanoTime();

            System.out.println("2. Dynamic Programming Algorithm");
            dpResult.forEach(System.out::println);
            long dpTime = t4 - t3;
            System.out.printf("Гүйцэтгэл: %,d ns%n", dpTime);
            System.out.println("--------------------------------\n");

            System.out.println("Гүйцэтгэлийн Харьцуулалт");
            System.out.printf("Greedy (T_G): %,d ns%n", greedyTime);
            System.out.printf("DP (T_DP): %,d ns%n", dpTime);

            if (dpTime > greedyTime)
                System.out.println("Дүгнэлт: Greedy илүү хурдан ажиллалаа.");
            else if (dpTime < greedyTime)
                System.out.println("Дүгнэлт: DP илүү хурдан ажиллалаа (санамсаргүй гүйцэтгэл).");
            else
                System.out.println("Дүгнэлт: Хоёр алгоритм ижил хугацаа зарцууллаа.");
        }
    }
}
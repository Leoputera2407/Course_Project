#### Code Cracker

In this project, we try to implement a code to decipher a cipher text. 

#### How it work

In cipher text, we know that stucture of words are retained, eventhough the letter are substituted. For example, `carrot` in cipher text could be `pittqe`. We could exploit this invariability to decrypt the cipher.

Thus, to solve this problem we compare to a corpus of words that we collected. However, to compare each word one by one is highly inefficient. So, we implemented a Hash map, which has a `O(1)` retrieval. 

We made the algorithm more efficient also by implementing the hashmap with a linkedlist. 

Then, we use recursion to slowly decipher the text.

| Score Attained |
| -------------- |
| 96/100         |






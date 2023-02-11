Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period?
   Answer:156.13 uA
   <br>Screenshot:  
   ![Avg_current_per_period][Avg_current_per_period]
   
2. What is the average current when the Si7021 is Powered Off?  
   Answer:1.97 uA
   <br>Screenshot:  
   ![Avg_current_lpmOFF][Avg_current_lpmOFF]
   
3. What is the average current when the Si7021 is Powered On?  
   Answer:4.64 mA
   <br>Screenshot:  
   ![Avg_current_lpmON][Avg_current_lpmON]
   
4. How long is the Si7021 Powered On for 1 temperature reading?  
   Answer:97 ms
   <br>Screenshot:  
   ![Time_lpmON][Time_lpmON]

[Avg_current_per_period]: screenshots/avgcyclecurrent.jpg "Avg_current_per_period"
[Avg_current_lpmOFF]: screenshots/average_current_power_off.jpg "Avg_current_lpmOFF"
[Avg_current_lpmON]: screenshots/avgcurrentonSi7021.jpg "Avg_current_lpmON"
[Time_lpmON]: screenshots/si7021ontime.jpg "Time_lpmON"
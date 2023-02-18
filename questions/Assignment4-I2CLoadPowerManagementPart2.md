Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period?
   Answer:17.37uA
   <br>Screenshot:  
   ![Avg_current_per_period][Avg_current_per_period]  

2. What is the average current when the Si7021 is Powered Off?
   Answer:2.43uA
   <br>Screenshot: 
   ![Avg_current_LPM_Off][Avg_current_LPM_Off]  

3. What is the average current when the Si7021 is Powered On?
   Answer:462.63uA
   <br>Screenshot:  
   ![Avg_current_LPM_On][Avg_current_LPM_On]

4. How long is the Si7021 Powered On for 1 temperature reading?
   Answer:97.90 ms
   <br>Screenshot:  
   ![duration_lpm_on][duration_lpm_on]  

5. Compute what the total operating time of your design for assignment 4 would be in hours, assuming a 1000mAh battery power supply?
   Answer:1000mAh*10^3/17.37 =  57570.523 hours
   
6. How has the power consumption performance of your design changed since the previous assignment?
   Answer:  In the previous assignment, we have not used sleep modes during I2C transfer and while counting the timer. We have used while loop for the last assignemnt which consumed a lot of power.
   In Assignement 4, we have used timerinterruptms by using COMP1 interrupt and we went to EM3 while the timer is running. During I2C transfer, We are in EM1 mode of operation and hence saved power. 
   
7. Describe how you tested your code for EM1 during I2C transfers.
   Answer: We can find the EM1 transition from the energy profiler. We can observe from the profile that the energy rises in after 80ms and then the energy drops which is due to the i2C_write_transfer() happening in EM1 mode.

[Avg_current_per_period]: screenshots/assignment4/avg_current_per_period.JPG "Avg_current_per_period"
[Avg_current_LPM_Off]: screenshots/assignment4/avg_current_lpm_off.JPG "Avg_current_LPM_Off"
[Avg_current_LPM_On]: screenshots/assignment4/avg_current_lpm_on.JPG "Avg_current_LPM_On"
[duration_lpm_on]:screenshots/assignment4/duration_lpm_on.JPG "duration_lpm_on"
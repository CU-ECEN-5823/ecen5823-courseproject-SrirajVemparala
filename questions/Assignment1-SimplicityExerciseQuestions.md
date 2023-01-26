Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**
   Answer: The instantaneous current drawn is 5.29mA.


**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**
   Answer: The instantaneous current drawn is 5.26mA. 


**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate. Extra credit is avilable for this question and depends on your answer.**
   Answer: No difference is observed. As per the data sheet the drive current is 1mA for Weak Drive and 10 mA for the strong drive current. We are checking current consumed by the LED and the current LED consumes nearly 1 mA which is a small value to distinguish using the strong drive and weak drive and hence the reading was same for strong and weak drive.


**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer:  4.93mA is the average current.


**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer:  5.12mA is the average current.



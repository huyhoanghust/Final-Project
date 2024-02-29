# Final-Project
Hướng dẫn cài đặt chương trình
A1. Cài đặt môi trường làm việc

A1.1 Cài đặt IDE Visual Studio Code
Visual Studio Code (VS Code) là một trình soạn thảo mã nguồn mở và miễn phí, được phát triển bởi Microsoft. Được thiết kế để hỗ trợ nhiều ngôn ngữ lập trình và nền tảng, VS Code đã trở thành một trong những trình soạn thảo mã nguồn phổ biến nhất trong cộng đồng phần mềm. 
Truy cập vào đường link sau: https://code.visualstudio.com/download, để tải môi trường lập trình IDE cho dự án. 

A1.2. Cài đặt Extensions cho dự án
Khi hoàn thành việc cài đặt IDE, tiếp theo là cài đặt Extensions để hỗ trợ lập trình cho chip STM32 với ngôn ngữ C/C++, các bước lần lượt như sau:

Bước 1: Cài đặt ngôn ngữ C/C++ cho môi trường
 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/c5723ca9-b5d8-49d0-95a0-e8a20d4d52d6)
 
Hình A- 1 Tiện ích cho ngôn ngữ C/C++

Bước 2: Cài đặt gói cho chip STM32
 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/815e266c-3108-4ea5-8891-b92105ee2d6e)
 
Hình A- 2 Tiện ích cho chip STM32

Bước 3: Cài đặt MakeFile để build chương trình
 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/ee57ca89-b4ee-4c80-89d3-e414a22be766)
 
Hình A- 3 Tiện ích cho Makefile

A1.3. Cài đặt Driver ST - Link hỗ trợ nạp chương trình
Sau khi thiết kế xong firmware, việc tiếp theo là nạp chương trình và debug thông qua Driver có sẵn do hãng ST của chip hỗ trợ. Đường dẫn để cài đặt Driver sau: https://www.st.com/en/development-tools/stsw-link009.html. 
A1.4. Viết file Json và Makefile cho dự án
Do Vscode là môi trường mở và đa nền tảng nên việc trích tới đường dẫn file build và chạy chương trình là cần thiết và do người dùng tự viết. Với dự án này em thiết kế như sau:
 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/d7a35f4a-14a7-47c9-ad6c-99cba1571213)
 
Hình A- 4 Thiết kế file Json cho trích xuất đường dẫn
 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/996a47c2-0fff-4ea4-98a0-36edac88ad6f)
 
Hình A- 5 Thiết kế Makefile để chạy chương trình

A2. Hướng dẫn sử dụng thiết bị
•	Bước 1: Lắp đặt các thành phần phụ kiện như Anten GPS, Anten 4G, các cảm biến, Pin RTC, màn hình Oled, quạt tản nhiệt vào thiết bị như Hình A- 6.

•	Bước 2: Cấp nguồn cho thiết bị bằng cách cắm nguồn Accqui từ xe vào thiết bị như Hình A- 7. 

•	Bước 3: Sau khi cấp nguồn, đợi khoảng 30 giây để thiết bị khởi động, kết nối mạng 4G, kết nối GPS. Nếu khởi động và kết nối thành công, đèn Led hiển thị trạng thái thiết bị bao gồm trạng thái làm việc vị trí số 1 (sáng liên tục), trạng thái sóng 4G vị trí số 2, GPS vị trí số 3 sẽ nhấp nháy với chu kì lần lượt là 0.5 giây và 1 giây như Hình A- 8.

•	Bước 4: Sau khi thấy Led hiển thị theo chu kì đó, nghĩa là thiết bị đã sẵn sàng để hoạt động trong suốt quá trình xe hoạt động. Thiết bị sẽ thu thập thông tin rồi xử lí sau đó gửi lên DashBoard để hiển thị và giám sát dữ liệu và thông tin như Hình A- 8. Nếu xe dừng hẳn sau 1 thời gian, thiết bị sẽ vào trạng thái ngủ đông để tiết kiệm năng lượng.

  ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/d8d9d3e5-0b68-47c7-939b-f954a203058e)
  ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/6c23b6c8-5d35-4775-a99c-d9a40fa2392c)
  
Hình A- 6 Lắp đặt các thành phần vào thiết bị

 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/878ae12d-6449-4d7b-a6ea-850c1c8dc371)
 
Hình A- 7 Phần cấp nguồn từ Accqui vào thiết bị

 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/ba3e85c6-6fa2-41ae-b40d-e9ae7586e70e)
 
Hình A- 9 Đèn Led hiển thị trạng thái

 ![image](https://github.com/huyhoanghust/Final-Project/assets/83686238/11c1e8b1-b9d4-4b04-b74c-680d8b6957d2)
 
Hình A- 8 Giao diện theo dõi thiết bị


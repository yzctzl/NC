# navicat-keygen for windows - 如何使用这个注册机？

1. 使用`navicat-patcher.exe`替换掉`navicat.exe`和`libcc.dll`里的Navicat激活公钥。 

   ```
   navicat-patcher.exe [-dry-run] <Navicat Install Path> [RSA-2048 PEM File Path]
   ```

   * `[-dry-run]`: 运行patcher但不对Navicat程序做任何修改。

     __这个参数是可选的。__

   * `<Navicat Install Path>`: Navicat的完整安装路径。 
     
     __这个参数必须指定。__

   * `[RSA-2048 PEM File Path]`: RSA-2048私钥文件的完整路径或相对路径。
     
     __这个参数是可选的。__ 如果未指定，`navicat-patcher.exe`将会在当前目录生成一个新的RSA-2048私钥文件。

   __例如：(在cmd.exe中)__ 

   ```
   navicat-patcher.exe "C:\Program Files\PremiumSoft\Navicat Premium 16"
   ```
   
   __Navicat Premium 16.0.7 英文版__ 已通过测试。下面将是一份样例输出：

   ```
   ***************************************************
   *       navicat-patcher by @DoubleLabyrinth       *
   *               version: 16.0.7.0                 *
   ***************************************************

   [+] Try to open libcc.dll ... OK!

   [*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_vtable = 0x00000001837759f0
   [*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = 0x0000000181fa52d0
   [*] patch_solution_since<16, 0, 7, 0>: m_va_iat_entry_malloc = 0x0000000183439bd0
   [+] patch_solution_since<16, 0, 7, 0>: official encoded key is found.

   [*] Generating new RSA private key, it may take a long time...
   [*] Your RSA private key:
   -----BEGIN RSA PRIVATE KEY-----
   MIIEpQIBAAKCAQEAvxaFFjTE6hi80nhjgfFMM3yPer122OIWIbbumFIuAOcCF6D3
   PnRHBdP9IqB99K6Nv6vKK3Jf0Y+dc5ETrg0l0AHYq+dTFTiWusHuRMx6xFjWzO96
   7mFmJq6P28dUucKnr6yG1TQeZaq+mHh2DNEnNEYgV7cLVT1unUmMOL/PBh/eCcaJ
   8hQNTQafQQknzCnAKC89v33y+rKInJNy9B+zSB0BGCz+eS8MKf6zc78JMSOnF2uj
   NK+QEwaYw8lAbJve1F+rCQS0mbm0QvHhZYZrblVHI5l/8LkX5qBtKw7duUhXHxmO
   fQieF23bBk9HDp5uQUGsdbKX6ZWitn/h926xyQIDAQABAoIBAQCHXxDRdni5zuSV
   xivYdnUhVHDg5zA23ZQINmw5BJ8KjJzy2FnPqNhXzKJb0Y7ptG8/BhinRtOSxkcp
   A/IJL89F2MkCn1JAimJd091UZ/fg+X7SmCVikyWm6auIa2IeZ0QcNAEhMVcHdzqn
   EU+wLMu1QKjQ+x/QN0ERtHTeDyQ+lUNB+bvAjx3LHN9Zh8weVBHHtwDoyyZDdJPw
   NWgpgcW+uYzlT66uh7LPPaRsEZgAkPIkhzZnwmugXdhlWxtYHKTEfe5gCqubQICc
   I/x1yBP1EZFm6qBQD4/49775ZbXwxgaWvBXG+Aah9x8JYtVUS4MgrAiC4a8NQqFp
   nwKVjUIBAoGBAOWsj9GGb2KYbfLzJNRrSxhs4TUBfpHteKSm2pL92NAbIOjssNhL
   hLY3gBFX2RnYmoGD6YT84JNykuAictgAd5GwvLIbaVF9l7MQn8APRbe2CzQ+/494
   9hpn33MZOBNd3I+a5+2qoFbXI04loyYDJkkeOqbwZzJjs7k9HmZMNwY5AoGBANT9
   tRFWFDvA0pPgGoHhzlsAUAmrbSfCPkhrRXpE9fgl3VnV+NRtjCf9NhJt0uaIokZ5
   oSf+jClcwU8N4EvGxMBaCHTqBzgc4dLPWpMAhPoMjjv1Oyug2iBcuTasHVP+Jdgq
   CaNzpXOuq4upaaNrq+QMsI6O9wA/zWhWPmnYQYgRAoGAUk56471noU+65zvXUQB6
   UvCB7Hrynt0ZRPg+kDrEPh/atV5NKdY2Yw6UqKJwvOBwzkU1pGDzIiQHGqd9vIa+
   Usmhdbp5DakSeitU9IEEnQdyEHEbKJFSsLfUzeyVuesDJbt/rh5dg4Fpt5GpW+/5
   Am8A2d6BPP+Z4qJSiJp7hZECgYEAy64TCZEXqEytE1yr/KjDfaK+54BX0j2e8gIj
   XtmznqoXE2Hboslfzp4Gp3j+xhbDmEGYK3bw8l0RP1g1tkFOxeNTUvq6DJ8SFVbV
   dt54S+bV3eCVxRL9hRUmyXGuWjQgXKdWsEhXYFkZE2Xe77h3mI3KCYoOCt74v146
   MV3szQECgYEAozTO7Wuum+VMKIY35hmHMjUiYmLl3EXWwMBT2VSsk8Siu0XoH0yd
   KoxsLDUBMS8sWKCZhFwU+Fx8UZjfo+xE3H4UTyVsw5EDpB9gSud928gNADwxTKor
   3s4jnUzb4XRQ0qN2jXzdNuqXNV1ozeqajbM2oSZqbSnWSs5g6DpIs1Q=
   -----END RSA PRIVATE KEY-----

   [*] patch_solution_since<16, 0, 7, 0>: Patch has been done.
   [*] New RSA-2048 private key has been saved to
       C:\Users\DoubleSine\source\repos\navicat-keygen\bin\x64-Release\RegPrivateKey.pem


   *******************************************************
   *           PATCH HAS BEEN DONE SUCCESSFULLY!         *
   *                  HAVE FUN AND ENJOY~                *
   *******************************************************
   ```

2. 接下来使用`navicat-keygen.exe`来生成序列号和激活码

   ```
   navicat-keygen.exe <-bin|-text> [-adv] <RSA-2048 Private Key File>
   ```

   * `<-bin|-text>`: 必须是`-bin`或`-text`。

     如果指定了`-bin`，`navicat-keygen.exe`最终将生成`license_file`文件。这个选项是给Navicat旧激活方式使用的。

     如果指定了`-text`，`navicat-keygen.exe`最终将生成Base64样式的激活码。这个选项是给Navicat新激活方式使用的。

     __这个参数必须指定。__

   * `[-adv]`: 开启高级模式。

     __这个参数是可选的。__ 如果指定了这个参数，`navicat-keygen.exe`将会要求你手工填写产品ID号、语言标识号。这个选项一般是给以后用的。

   * `<RSA-2048 Private Key File>`: RSA-2048私钥文件的完整路径或相对路径。私钥必须是PEM格式的。
     
     __这个参数必须指定。__

   __例如：(在cmd.exe中)__ 

   ```console
   navicat-keygen.exe -text .\RegPrivateKey.pem
   ```

   你会被要求选择Navicat产品类别、语言以及输入主版本号。之后会随机生成一个序列号。

   ```
   ***************************************************
   *       navicat-keygen by @DoubleLabyrinth        *
   *                version: 16.0.7.0                *
   ***************************************************

   [*] Select Navicat product:
   0. DataModeler
   1. Premium
   2. MySQL
   3. PostgreSQL
   4. Oracle
   5. SQLServer
   6. SQLite
   7. MariaDB
   8. MongoDB
   9. ReportViewer

   (Input index)> 1

   [*] Select product language:
   0. English
   1. Simplified Chinese
   2. Traditional Chinese
   3. Japanese
   4. Polish
   5. Spanish
   6. French
   7. German
   8. Korean
   9. Russian
   10. Portuguese

   (Input index)> 0

   [*] Input major version number:
   (range: 11 ~ 16, default: 16)> 16

   [*] Serial number:
   NAVL-GFKA-T5SR-ZFTK

   [*] Your name:
   ```

   你可以使用这个序列号暂时激活Navicat。

   接下来你会被要求输入`用户名`和`组织名`；请随便填写，但不要太长。

   ```
   [*] Your name: Double Sine
   [*] Your organization: PremiumSoft CyberTech Ltd.

   [*] Input request code (in Base64), input empty line to end:
   ```
 
   之后你会被要求填入请求码。注意 __不要关闭命令行__.

3. __断开网络__ 并打开Navicat。找到`注册`窗口，并填入keygen给你的序列号。然后点击`激活`按钮。

4. 一般来说在线激活肯定会失败，这时候Navicat会询问你是否`手动激活`，直接选吧。

5. 在`手动激活`窗口你会得到一个请求码，复制它并把它粘贴到keygen里。最后别忘了连按至少两下回车结束输入。

   ```
   [*] Your name: Double Sine
   [*] Your organization: PremiumSoft CyberTech Ltd.

   [*] Input request code (in Base64), input empty line to end:
   CpgnfbIJGmAcxCuo/pAb8EeoS0audZn2NNemg6c3NPK/dWgb343IZQrFwoBZY6lpxE4Fq1BoNmCM75P03XpiXQ+hErcvFWk6iQPDCk/d4msf/AoprIqAMpXFoFLkeP0G93UIIEeBsUej8SrxdDgQDM585iPok5fUW+fTDCD1VICr7DBdL3c/69IxeIgiOQSuImdIQiM3/EOfDiFbAJL9vHW5LxFT8jj+8RPXehwPTBphpInmGdzxVZUZJwAGlXt7orrRbzafdeBjz6MnTajTcJP3SS2dBCiR33UScnyxYGEXdzv7+QLScTmCvI7gqg3Z8DMhroKMoHmy1AvC16FKVw==

   [*] Request Info:
   {"K":"NAVLGFKAT5SRZFTK", "DI":"7D48FCBD093C778879A1", "P":"WIN"}

   [*] Response Info:
   {"K":"NAVLGFKAT5SRZFTK","DI":"7D48FCBD093C778879A1","N":"Double Sine","O":"PremiumSoft CyberTech Ltd.","T":1644387294}

   [*] Activation Code:
   vwLGmQIWg/DtzHMcaKCDHAjTcBNbTo2VmNllphUSUMgGjgvL6v82ue+GqXB6M/qn48Rj4D4Joqqisr6UwMSclNmQxOQz4RftEpLtG6KBjDo4LM71qn9R/jWoZV5EoHPQkX5gzhO/D7GammrRGn2MV+zI6dJ4c4SBFNnNyjAeEqNzinrQwjB7lUVTlpHEe/SMrdCsGliPZQ/X+5ASbEsq3D8PZsjysJv98MIJrZvdTdznrRe8JzYP+8sbIPQMIX1UDmdyDpbpSl45N92OhO4htz1kFjUEfnrwY0GMOhdYHv/PfMI7RiQzkRyY7pLvX7muJ4dkA+CmMmwew3gy3MWjig==
   ```

6. 如果不出意外，你会得到一个看似用Base64编码的激活码。直接复制它，并把它粘贴到Navicat的`手动激活`窗口，最后点`激活`按钮。如果没什么意外的话应该能成功激活。


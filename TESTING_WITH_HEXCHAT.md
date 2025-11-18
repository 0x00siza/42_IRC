# Testing Your Operator Commands with HexChat

## 🎯 Quick Test Guide

### Prerequisites
✅ Server is running: `./ircserv 6667 password123`
✅ HexChat is connected
✅ You're registered (NICK and USER commands done)

---

## 📝 Step-by-Step Testing

### **Step 1: Join a Channel (You become operator)**

In HexChat, type:
```
/join #mytest
```

✅ **What to check:**
- You should see yourself join the channel
- You should have `@` symbol before your name (means you're operator)
- First person to join always becomes operator

---

### **Step 2: Get Another Client** 

**Option A: Use Another HexChat Window**
- File → New Server Window
- Connect to 127.0.0.1:6667
- Use different nickname (e.g., "bob")
- Join the same channel: `/join #mytest`

**Option B: Use netcat in Terminal**
```bash
# In a new terminal
nc 127.0.0.1 6667
PASS password123
NICK bob
USER bob 0 * :Bob Smith
JOIN #mytest
```

---

## 🧪 Test Each Command

### **TEST 1: MODE Command (View Modes)**

In HexChat, type:
```
/mode #mytest
```

✅ **Expected:** Shows current channel modes (probably just `+t` by default)

---

### **TEST 2: MODE +i (Invite-Only)**

```
/mode #mytest +i
```

✅ **Expected:** 
- You see: `MODE #mytest +i`
- Channel is now invite-only
- Others can't join without invite

**Verify it works:**
- Try joining with a 3rd client → Should get error 473 "Cannot join channel (+i)"

---

### **TEST 3: MODE +t (Topic Restricted)**

```
/mode #mytest +t
```

✅ **Expected:**
- Topic can only be changed by operators
- Regular members get error 482 if they try

---

### **TEST 4: MODE +k (Channel Password)**

```
/mode #mytest +k secretpass
```

✅ **Expected:**
- Channel now requires password "secretpass" to join
- You see: `MODE #mytest +k secretpass`

**Verify it works:**
```
# Try joining without password
/join #newchan
/part #newchan
/join #mytest
# Should fail with error 475 "Cannot join channel (+k)"

# Join with password
/join #mytest secretpass
# Should work!
```

**Remove password:**
```
/mode #mytest -k
```

---

### **TEST 5: MODE +l (User Limit)**

```
/mode #mytest +l 3
```

✅ **Expected:**
- Channel limited to 3 users
- 4th person trying to join gets error 471 "Cannot join channel (+l)"

**Remove limit:**
```
/mode #mytest -l
```

---

### **TEST 6: MODE +o (Give Operator)**

First, make sure you have another user in the channel (bob). Then:

```
/mode #mytest +o bob
```

✅ **Expected:**
- You see: `MODE #mytest +o bob`
- Bob now has `@` symbol (is operator)
- Bob can now use operator commands

**Take away operator:**
```
/mode #mytest -o bob
```

---

### **TEST 7: TOPIC Command**

**View topic:**
```
/topic #mytest
```

**Set topic:**
```
/topic #mytest Welcome to my test channel!
```

✅ **Expected:**
- Topic changes for everyone in channel
- Everyone sees: `TOPIC #mytest :Welcome to my test channel!`

**Test +t restriction:**
```
# Make sure +t is set
/mode #mytest +t

# Try changing topic as non-operator (bob)
# Bob types: /topic #mytest Bob's topic
# Bob should get: error 482 "You're not channel operator"
```

---

### **TEST 8: INVITE Command**

```
/invite charlie #mytest
```

✅ **Expected:**
- You see: `341 yournick charlie #mytest` (confirmation)
- Charlie receives invite notification
- If channel is +i, charlie can now join

**Test with +i mode:**
```
# Set invite-only
/mode #mytest +i

# Invite someone
/invite charlie #mytest

# Charlie can join now, others can't
```

---

### **TEST 9: KICK Command**

```
/kick #mytest bob Stop spamming!
```

✅ **Expected:**
- You see: `KICK #mytest bob :Stop spamming!`
- Bob is removed from channel
- Bob sees the kick message
- Everyone in channel sees bob was kicked

**Kick without reason:**
```
/kick #mytest bob
```
(Default reason: "No reason given")

**Test permission:**
- Bob tries to kick you → Should get error 482 "You're not channel operator"

---

### **TEST 10: Multiple Modes at Once**

```
/mode #mytest +itk mypassword
```

✅ **Expected:**
- Sets +i (invite-only)
- Sets +t (topic-restricted)  
- Sets +k mypassword (password)
- All applied at once

**Give multiple operators:**
```
/mode #mytest +oo bob charlie
```

---

## 🔍 How to See It's Working

### **In HexChat Window:**

1. **Server messages** appear in the channel/server tab
2. **Mode changes** show as: `* yournick sets mode +i`
3. **Kicks** show as: `* bob was kicked by yournick (reason)`
4. **Topics** show as: `* yournick changes topic to "..."`
5. **Invites** show in the server tab

### **Look for These Confirmations:**

✅ **MODE works:** You see the mode change message
✅ **KICK works:** User disappears from channel, kick message shows
✅ **INVITE works:** You get numeric 341 confirmation
✅ **TOPIC works:** Topic line at top of channel changes

### **Error Messages (These are GOOD - they mean protection works):**

- `482 #channel :You're not channel operator` - Non-op tried operator command ✅
- `473 #channel :Cannot join channel (+i)` - Tried to join invite-only without invite ✅
- `475 #channel :Cannot join channel (+k)` - Wrong password ✅
- `471 #channel :Cannot join channel (+l)` - Channel full ✅
- `442 #channel :You're not on that channel` - Tried command on channel you're not in ✅

---

## 🎬 Quick Test Scenario

Here's a complete test you can run in 2 minutes:

```irc
# 1. Join and become operator
/join #test

# 2. Set multiple modes
/mode #test +itk secretpass

# 3. Check modes were set
/mode #test
# Should show: +itk

# 4. Change topic
/topic #test Welcome everyone!

# 5. Have someone else join with password
# (from another client): /join #test secretpass

# 6. Give them operator
/mode #test +o bob

# 7. Kick them (testing)
/kick #test bob Testing kick feature

# 8. Invite them back
/invite bob #test

# 9. Remove modes
/mode #test -itk

# 10. Check modes cleared
/mode #test
# Should show: + or just +t
```

---

## 🐛 Troubleshooting

### "Nothing happens when I type command"
- Check server terminal - are errors printed?
- Make sure you're in the channel
- Make sure you have operator (@) status

### "Command not found"
- Your teammates may not have connected the command handlers yet
- Check if KICK/INVITE/TOPIC/MODE are in the command dispatcher

### "Permission denied"
- Make sure you have `@` before your name
- Only operators can use these commands
- First person to join becomes operator

### "Server crashes"
- Check server terminal for error messages
- Look for segmentation faults
- May need debugging

---

## 📊 Success Checklist

Test each command and check it off:

- [ ] **MODE +i** - Invite-only works, non-invited users can't join
- [ ] **MODE +t** - Topic restriction works, non-ops can't change topic
- [ ] **MODE +k** - Password works, wrong password denied
- [ ] **MODE +o** - Operator given, new op can use commands
- [ ] **MODE +l** - User limit works, blocks when full
- [ ] **MODE view** - Shows current modes correctly
- [ ] **TOPIC view** - Shows current topic
- [ ] **TOPIC set** - Changes topic, broadcasts to all
- [ ] **INVITE** - Invites user, works with +i mode
- [ ] **KICK** - Removes user, shows reason, broadcasts

If all 10 are checked, **your operator commands are working perfectly!** ✅

---

## 💡 Pro Tips

1. **Use HexChat's raw log** to see actual IRC messages:
   - Window → Raw Log
   - See exactly what server sends/receives

2. **Test permissions:** Have non-operators try commands (should fail)

3. **Test edge cases:**
   - Kick non-existent user
   - Invite to non-existent channel
   - Set mode on channel you're not in
   - Use MODE on channel you're not operator of

4. **Multiple modes:** Test combining modes like `+itl 50`

5. **Check broadcasts:** When you kick/mode/topic, everyone should see it

---

## 🎉 Your Commands Are Working If:

✅ You can set and remove all 5 modes (i, t, k, o, l)
✅ Non-operators get "permission denied" errors
✅ KICK removes users and broadcasts message
✅ INVITE adds to invite list and works with +i
✅ TOPIC changes are visible to everyone
✅ MODE changes are broadcast to channel members

**Good luck with testing!** 🚀

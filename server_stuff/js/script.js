const funFunction = () => {
  const cheeKit = document.getElementsByClassName("chee-kit");
  for (let item of cheeKit) {
    const fontSize = parseInt(
      window.getComputedStyle(item, null).getPropertyValue("font-size")
    );
    item.style.fontSize = fontSize + 10 + "px";
  }
  var element = document.body;
  element.classList.toggle("dark-mode");
};
